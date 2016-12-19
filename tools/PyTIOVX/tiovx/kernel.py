'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class KernelParams :
    def __init__(self, index, type, direction, state, name, do_map=True, do_unmap=True, do_map_unmap_all_planes=False):
        self.index = index
        self.type = type
        self.direction = direction
        self.state = state
        self.name_upper = name.upper()
        self.name_lower = name.lower()
        self.name_camel = toCamelCase(name)
        self.do_map = do_map
        self.do_unmap = do_unmap
        self.do_map_unmap_all_planes = do_map_unmap_all_planes
        if Type.is_scalar_type(type) :
            self.do_map = False;
            self.do_unmap = False;

    def __str__(self):
        return "Param " + str(self.index) + ": " + self.name_upper + " " + Type.get_vx_enum_name(self.type) + " " + Direction.get_vx_enum_name(self.direction) + " " + ParamState.get_vx_enum_name(self.state)

class Kernel  :
    def __init__(self, name="default") :
        self.name_lower = name.lower()
        self.name_upper = name.upper()
        self.name_camel = toCamelCase(name)
        self.index = 0;
        self.params = []
        self.name_str_prefix = "org.khronos.openvx."
        self.enum_str_prefix = "VX_KERNEL_"
        self.targets = []

    def setKernelPrefix(self, name_str_prefix, enum_str_prefix) :
        self.name_str_prefix = name_str_prefix
        self.enum_str_prefix = enum_str_prefix

    def setTarget(self, target) :
        self.targets.append(target)

    def __str__(self) :
        kernel_str = "Kernel: " + self.name_lower + " "+ self.name_upper + " "+ self.name_camel + "\n"
        kernel_str += "Targets: "
        for target in self.targets :
            kernel_str += Target.get_vx_enum_name(target) + "(CPU: " + Cpu.get_vx_enum_name(Target.get_cpu(target)) + ") "
        kernel_str += "\n"
        for prm in self.params :
            kernel_str += str(prm) + "\n"
        return kernel_str

    def setParameter(self, type, direction, state, name, do_map=True, do_unmap=True, do_map_unmap_all_planes=False):
        params = KernelParams(self.index, type, direction, state, name, do_map, do_unmap, do_map_unmap_all_planes);
        self.params.append(params)
        self.index = self.index + 1



