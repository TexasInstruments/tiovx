'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

context = Context("uc_sample_02")
graph = Graph()

in1 = Image(640, 480, DfImage.NV12)
in2 = Image(640, 480, DfImage.NV12)
out1 = Image(640, 480, DfImage.NV12)
out2 = Image(640, 480, DfImage.NV12)
out3 = Image(640, 480, DfImage.NV12)

graph.add ( NodeAbsDiff(in1, in2, out1, target=Target.DSP1) )
graph.add ( NodeAbsDiff(in1, in2, out2, target=Target.EVE1) )
graph.add ( NodeAbsDiff(in1, in2, out3, target=Target.MPU_0) )

context.add ( graph )

ExportImage(context).export()
