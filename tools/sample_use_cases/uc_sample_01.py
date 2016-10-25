'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

context = Context("uc_sample_01")

in1 = Image(640, 480, DfImage.NV12)
in2 = Image(640, 480, DfImage.NV12)
out = Image(640, 480, DfImage.NV12)

context.add ( NodeAbsDiff(in1, in2, out) )

ExportImage(context).export()
