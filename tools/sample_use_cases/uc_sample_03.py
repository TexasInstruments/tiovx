'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

context = Context("uc_sample_03")
graph = Graph()

in1 = Image(640, 480, DfImage.S16)
in2 = Image(640, 480, DfImage.S16)
out = Image(640, 480, DfImage.S16)

graph.add ( NodeAbsDiff(in1, in2, out) )

context.add ( graph )

ExportImage(context).export()
ExportCode(context).export()
