'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''
from tiovx import *

class NodePhaseRgb (Node) :
    def __init__(self, image_in, image_out, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "vx_tutorial_graph.phase_rgb", image_in, image_out)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_USER_KERNEL")

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].width    == self.ref[1].width ), "Input and Output width MUST match"
        assert ( self.ref[0].height   == self.ref[1].height ), "Input and Output height MUST match"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input data format must be U8"
        assert ( self.ref[1].df_image == DfImage.RGB ), "Output data format must be RGB"


context = Context("vx_tutorial_graph_user_kernel_pytiovx_uc")
graph = Graph()

width = 640
height = 480

in_image = Image(width, height, DfImage.U8, name="input")
grad_x = Image(width, height, DfImage.S16, name="grad_x")
grad_y = Image(width, height, DfImage.S16, name="grad_y")
phase = Image(width, height, DfImage.U8, name="phase")
phase_rgb = Image(width, height, DfImage.RGB, name="phase_rgb")

graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )
graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )
graph.add ( NodePhaseRgb(phase, phase_rgb, target=Target.DSP1) )

context.add ( graph )

ExportImage(context).export()
ExportCode(context).export()
