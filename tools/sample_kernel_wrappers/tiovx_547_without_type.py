from tiovx import *
code = KernelExportCode(Module.IMAGING, Core.C66, "CUSTOM_APPLICATION_PATH")
kernel = Kernel("fast_corners")
kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "input")
kernel.setParameter(Type.ARRAY, Direction.OUTPUT, ParamState.REQUIRED, "corners")
kernel.setTarget(Target.DSP1)
code.export(kernel)
code.exportDiagram(kernel)
