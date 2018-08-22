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

# this will fail due to having a keyword used by arrays
kernel.allocateLocalMemory("img_scratch_mem", ["offset"], "IN_IMAGE")

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
