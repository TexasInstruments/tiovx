'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

code = KernelExportCode("test", Core.ARM, "CUSTOM_APPLICATION_PATH")

kernel = Kernel("pyramid_intermediate")

kernel.setParameter(Type.PYRAMID, Direction.INPUT, ParamState.REQUIRED, "INPUT", ['VX_DF_IMAGE_U8'])
kernel.setParameter(Type.PYRAMID, Direction.OUTPUT, ParamState.REQUIRED, "OUTPUT", ['VX_DF_IMAGE_U8'])

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

code.export(kernel)
