'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode("IMAGING", Core.C66, "CUSTOM_APPLICATION_PATH")

kernel = Kernel("tensor_test")

kernel.setParameter(Type.TENSOR, Direction.INPUT, ParamState.REQUIRED, "INPUT")
kernel.setParameter(Type.TENSOR, Direction.OUTPUT, ParamState.REQUIRED, "OUTPUT")

kernel.setTarget(Target.MCU2_0)
code.export(kernel)


