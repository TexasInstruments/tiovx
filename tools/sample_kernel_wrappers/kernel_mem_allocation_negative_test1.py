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

# null allocation
kernel.allocateLocalMemory("scratch_mem", ["2*4"])

# this will fail due to the name being the same as before
kernel.allocateLocalMemory("scratch_mem", ["3*4"])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
