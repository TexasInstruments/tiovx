'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode(Module.TEST_KERNELS, Core.C66, "CUSTOM_KERNEL_PATH")

kernel = Kernel("not_not")

kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "INPUT", ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUTPUT", ['VX_DF_IMAGE_U8'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
