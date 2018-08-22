'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode("superset_module", Core.C66, "CUSTOM_APPLICATION_PATH")

kernel = Kernel("kernel_mem_allocation")

# image
kernel.setParameter(Type.IMAGE,   Direction.INPUT, ParamState.REQUIRED, "IN_IMAGE", ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.IMAGE,   Direction.OUTPUT, ParamState.OPTIONAL, "IN_IMAGE_OPT", ['VX_DF_IMAGE_U8'])

# array
kernel.setParameter(Type.ARRAY,   Direction.INPUT, ParamState.REQUIRED, "IN_ARRAY")
kernel.setParameter(Type.ARRAY,   Direction.OUTPUT, ParamState.OPTIONAL, "IN_ARRAY_OPT")

# pyramid
kernel.setParameter(Type.PYRAMID, Direction.INPUT, ParamState.REQUIRED, "IN_PYRAMID",  ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.PYRAMID, Direction.OUTPUT, ParamState.OPTIONAL, "IN_PYRAMID_OPT",  ['VX_DF_IMAGE_U8'])

# matrix
kernel.setParameter(Type.MATRIX,  Direction.INPUT, ParamState.REQUIRED, "IN_MATRIX", ['VX_DF_IMAGE_S16'])
kernel.setParameter(Type.MATRIX,  Direction.OUTPUT, ParamState.OPTIONAL, "IN_MATRIX_OPT", ['VX_DF_IMAGE_S16'])

# distribution
kernel.setParameter(Type.DISTRIBUTION,  Direction.INPUT, ParamState.REQUIRED, "IN_DISTRIBUTION",  ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.DISTRIBUTION,  Direction.OUTPUT, ParamState.OPTIONAL, "IN_DISTRIBUTION_OPT",  ['VX_DF_IMAGE_U8'])

# lut
kernel.setParameter(Type.LUT, Direction.INPUT, ParamState.REQUIRED, "IN_LUT", ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.LUT, Direction.OUTPUT, ParamState.OPTIONAL, "IN_LUT_OPT", ['VX_DF_IMAGE_U16'])

# remap
kernel.setParameter(Type.REMAP, Direction.INPUT, ParamState.REQUIRED, "IN_REMAP", ['VX_DF_IMAGE_U16'])
kernel.setParameter(Type.REMAP, Direction.OUTPUT, ParamState.OPTIONAL, "IN_REMAP_OPT", ['VX_DF_IMAGE_U16'])

# convolution
kernel.setParameter(Type.CONVOLUTION,   Direction.INPUT, ParamState.REQUIRED, "IN_CONVOLUTION",   ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.CONVOLUTION,   Direction.OUTPUT, ParamState.OPTIONAL, "IN_CONVOLUTION_OPT",   ['VX_DF_IMAGE_U8'])

# object array
kernel.setParameter(Type.OBJECT_ARRAY,  Direction.INPUT, ParamState.REQUIRED, "IN_OBJECT_ARRAY",  ['VX_DF_IMAGE_S32'])
kernel.setParameter(Type.OBJECT_ARRAY,  Direction.OUTPUT, ParamState.OPTIONAL, "IN_OBJECT_ARRAY_OPT",  ['VX_DF_IMAGE_S32'])

# required img
kernel.allocateLocalMemory("img_scratch_mem", ["width*height*stride_y*stride_x"], "IN_IMAGE")
kernel.allocateLocalMemory("img_scratch_mem2", [Attribute.Image.WIDTH, Attribute.Image.HEIGHT], "IN_IMAGE")

# optional img
kernel.allocateLocalMemory("img_opt_scratch_mem", ["width*height*stride_y*stride_x"], "IN_IMAGE_OPT")
kernel.allocateLocalMemory("img_opt_scratch_mem2", [Attribute.Image.WIDTH, Attribute.Image.HEIGHT], "IN_IMAGE_OPT")

# required pyr
kernel.allocateLocalMemory("in_pyramid_scratch_mem", ["levels"], "IN_PYRAMID")
kernel.allocateLocalMemory("in_pyramid_scratch_mem2", [Attribute.Pyramid.LEVELS], "IN_PYRAMID")

# optional pyr
kernel.allocateLocalMemory("in_pyramid_opt_scratch_mem", ["levels"], "IN_PYRAMID_OPT")
kernel.allocateLocalMemory("in_pyramid_opt_scratch_mem2", [Attribute.Pyramid.LEVELS], "IN_PYRAMID_OPT")

# required mat
kernel.allocateLocalMemory("in_mat_scratch_mem", ["rows*columns*size"], "IN_MATRIX")
kernel.allocateLocalMemory("in_mat_scratch_mem2", [Attribute.Matrix.ROWS, Attribute.Matrix.COLUMNS, Attribute.Matrix.SIZE], "IN_MATRIX")

# optional mat
kernel.allocateLocalMemory("in_mat_opt_scratch_mem", ["rows*columns*size"], "IN_MATRIX_OPT")
kernel.allocateLocalMemory("in_mat_opt_scratch_mem2", [Attribute.Matrix.ROWS, Attribute.Matrix.COLUMNS, Attribute.Matrix.SIZE], "IN_MATRIX_OPT")

# required arr
kernel.allocateLocalMemory("in_array_scratch_mem", ["capacity*itemsize*numitems*itemtype"], "IN_ARRAY")
kernel.allocateLocalMemory("in_array_scratch_mem2", [Attribute.Array.CAPACITY, Attribute.Array.ITEMSIZE, Attribute.Array.NUMITEMS, Attribute.Array.ITEMTYPE], "IN_ARRAY")

# optional arr
kernel.allocateLocalMemory("in_array_opt_scratch_mem", ["capacity*itemsize*numitems*itemtype"], "IN_ARRAY_OPT")
kernel.allocateLocalMemory("in_array_opt_scratch_mem2", [Attribute.Array.CAPACITY, Attribute.Array.ITEMSIZE, Attribute.Array.NUMITEMS, Attribute.Array.ITEMTYPE], "IN_ARRAY_OPT")

# required dist
kernel.allocateLocalMemory("in_dist_scratch_mem", ["dimensions*offset*range*bins*window*size"], "IN_DISTRIBUTION")
kernel.allocateLocalMemory("in_dist_scratch_mem2", [Attribute.Distribution.DIMENSIONS, Attribute.Distribution.OFFSET, Attribute.Distribution.RANGE, Attribute.Distribution.BINS, Attribute.Distribution.WINDOW, Attribute.Distribution.SIZE], "IN_DISTRIBUTION")

# optional dist
kernel.allocateLocalMemory("in_dist_opt_scratch_mem", ["dimensions*offset*range*bins*window*size"], "IN_DISTRIBUTION_OPT")
kernel.allocateLocalMemory("in_dist_opt_scratch_mem2", [Attribute.Distribution.DIMENSIONS, Attribute.Distribution.OFFSET, Attribute.Distribution.RANGE, Attribute.Distribution.BINS, Attribute.Distribution.WINDOW, Attribute.Distribution.SIZE], "IN_DISTRIBUTION_OPT")

# required lut
kernel.allocateLocalMemory("in_lut_scratch_mem", ["count*size"], "IN_LUT")
kernel.allocateLocalMemory("in_lut_scratch_mem2", [Attribute.Lut.COUNT, Attribute.Lut.SIZE], "IN_LUT")

# optional lut
kernel.allocateLocalMemory("in_lut_opt_scratch_mem", ["count*size"], "IN_LUT_OPT")
kernel.allocateLocalMemory("in_lut_opt_scratch_mem2", [Attribute.Lut.COUNT, Attribute.Lut.SIZE], "IN_LUT_OPT")

# required remap
kernel.allocateLocalMemory("in_remap_scratch_mem", ["source_width*source_height*destination_width*destination_height"], "IN_REMAP")
kernel.allocateLocalMemory("in_remap_scratch_mem2", [Attribute.Remap.SOURCE_WIDTH, Attribute.Remap.SOURCE_HEIGHT, Attribute.Remap.DESTINATION_WIDTH, Attribute.Remap.DESTINATION_HEIGHT], "IN_REMAP")

# optional remap
kernel.allocateLocalMemory("in_remap_opt_scratch_mem", ["source_width*source_height*destination_width*destination_height"], "IN_REMAP_OPT")
kernel.allocateLocalMemory("in_remap_opt_scratch_mem2", [Attribute.Remap.SOURCE_WIDTH, Attribute.Remap.SOURCE_HEIGHT, Attribute.Remap.DESTINATION_WIDTH, Attribute.Remap.DESTINATION_HEIGHT], "IN_REMAP_OPT")

# required conv
kernel.allocateLocalMemory("in_conv_scratch_mem", ["rows*columns*size*scale"], "IN_CONVOLUTION")
kernel.allocateLocalMemory("in_conv_scratch_mem2", [Attribute.Convolution.ROWS, Attribute.Convolution.COLUMNS, Attribute.Convolution.SCALE, Attribute.Convolution.SIZE], "IN_CONVOLUTION")

# optional conv
kernel.allocateLocalMemory("in_conv_opt_scratch_mem", ["rows*columns*size*scale"], "IN_CONVOLUTION_OPT")
kernel.allocateLocalMemory("in_conv_opt_scratch_mem2", [Attribute.Convolution.ROWS, Attribute.Convolution.COLUMNS, Attribute.Convolution.SCALE, Attribute.Convolution.SIZE], "IN_CONVOLUTION_OPT")

# required obj arr
kernel.allocateLocalMemory("in_obj_arr_scratch_mem", ["numitems"], "IN_OBJECT_ARRAY")
kernel.allocateLocalMemory("in_obj_arr_scratch_mem2", [Attribute.ObjectArray.NUMITEMS], "IN_OBJECT_ARRAY")

# optional conv
kernel.allocateLocalMemory("in_obj_arr_opt_scratch_mem", ["numitems"], "IN_OBJECT_ARRAY_OPT")
kernel.allocateLocalMemory("in_obj_arr_opt_scratch_mem2", [Attribute.ObjectArray.NUMITEMS], "IN_OBJECT_ARRAY_OPT")

# null allocation
kernel.allocateLocalMemory("scratch_mem", ["2*4"])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
code.exportDiagram(kernel)
