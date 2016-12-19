'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from tiovx import *

context = Context("uc_sample_06")

sc1 = Scalar(Type.CHAR, 'c')
sc2 = Scalar(Type.INT8, -0x12)
sc3 = Scalar(Type.UINT8, 0x12)
sc4 = Scalar(Type.INT16, -0x1234)
sc5 = Scalar(Type.UINT16, 0x1234)
sc6 = Scalar(Type.INT32, -0x12345678)
sc7 = Scalar(Type.UINT32, 0x12345678)
sc8 = Scalar(Type.INT64, -0x1234567890)
sc9 = Scalar(Type.UINT64, 0x1234567890)
sc10 = Scalar(Type.FLOAT32, -1234.1234)
sc11 = Scalar(Type.FLOAT64, -1234567890.1234567890)
sc12 = Scalar(Type.ENUM, Type.FLOAT32)
sc13 = Scalar(Type.SIZE, 1234)
sc14 = Scalar(Type.DF_IMAGE, DfImage.NV12)
sc15 = Scalar(Type.BOOL, True)

context.add ( sc1 )
context.add ( sc2 )
context.add ( sc3 )
context.add ( sc4 )
context.add ( sc5 )
context.add ( sc6 )
context.add ( sc7 )
context.add ( sc8 )
context.add ( sc9 )
context.add ( sc10 )
context.add ( sc11 )
context.add ( sc12 )
context.add ( sc13 )
context.add ( sc14 )
context.add ( sc15 )

ExportCode(context).export()
