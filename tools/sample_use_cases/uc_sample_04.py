'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

'''
This is test case to test error detection
'''

from tiovx import *

context = Context("uc_sample_04")
graph = Graph()

in1 = Image(640, 480, DfImage.U8)
in2 = Lut(Type.INT16, 256)
out = Image(640, 480, DfImage.U8)

graph.add ( NodeAbsDiff(in1, in2, out) )

context.add ( graph )

ExportImage(context).export()
