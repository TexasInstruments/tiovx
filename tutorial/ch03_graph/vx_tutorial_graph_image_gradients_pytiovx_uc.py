'''
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''
from tiovx import *

context = Context("vx_tutorial_graph_image_gradients_pytiovx_uc")
graph = Graph()

width = 640
height = 480

in_image = Image(width, height, DfImage.U8, name="input")
grad_x = Image(width, height, DfImage.S16, name="grad_x")
grad_y = Image(width, height, DfImage.S16, name="grad_y")
magnitude = Image(width, height, DfImage.S16, name="magnitude")
phase = Image(width, height, DfImage.U8, name="phase")
grad_x_img = Image(width, height, DfImage.U8, name="grad_x_img")
grad_y_img = Image(width, height, DfImage.U8, name="grad_y_img")
magnitude_img = Image(width, height, DfImage.U8, name="magnitude_img")
shift = Scalar(Type.INT32, 0, name="shift")

graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )
graph.add ( NodeMagnitude(grad_x, grad_y, magnitude, target=Target.DSP2) )
graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )
graph.add ( NodeConvertDepth(magnitude, magnitude_img, Policy.SATURATE, shift, target=Target.DSP2) )
graph.add ( NodeConvertDepth(grad_x, grad_x_img, Policy.SATURATE, shift, target=Target.DSP2) )
graph.add ( NodeConvertDepth(grad_y, grad_y_img, Policy.SATURATE, shift, target=Target.DSP1) )

context.add ( graph )

ExportImage(context).export()
ExportCode(context).export()
