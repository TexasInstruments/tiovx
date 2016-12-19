'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

kernel = Kernel("channel_combine")

kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "SRC0")
kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "SRC1")
kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.OPTIONAL, "SRC2")
kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.OPTIONAL, "SRC3")
kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "DST", do_map_umnap_all_planes=True)

kernel.setTarget(Target.DSP1)
kernel.setTarget(Target.DSP2)

KernelExportCode(kernel).export();