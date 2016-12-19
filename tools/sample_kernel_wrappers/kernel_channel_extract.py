'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

kernel = Kernel("channel_extract")

kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "IN", do_map=False, do_unmap=False)
kernel.setParameter(Type.ENUM, Direction.INPUT, ParamState.REQUIRED, "CHANNEL")
kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUT")

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

KernelExportCode(kernel).export();