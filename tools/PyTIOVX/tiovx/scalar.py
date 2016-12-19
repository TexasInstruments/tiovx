'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Scalar (Reference) :
    def __init__(self, data_type, value="0", name="default") :
        Reference.__init__(self, Type.SCALAR, name)
        self.data_type = data_type
        self.value = value
        self.check_value()
        self.get_value_str()

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ' ]'

    def check_value(self) :
        if self.data_type == Type.CHAR :
            return
        if self.data_type == Type.INT8 :
            assert ( self.value > -255 and self.value <= 255)
            return

    def get_value_str(self):
        if self.data_type == Type.CHAR :
            return "'" + str(self.value) + "'"
        if self.data_type == Type.INT8 :
            return str(self.value)
        if self.data_type == Type.UINT8 :
            return str(self.value)
        if self.data_type == Type.INT16 :
            return str(self.value)
        if self.data_type == Type.UINT16 :
            return str(self.value)
        if self.data_type == Type.INT32 :
            return str(self.value)
        if self.data_type == Type.UINT32 :
            return str(self.value)
        if self.data_type == Type.INT64 :
            return str(self.value)
        if self.data_type == Type.UINT64 :
            return str(self.value)
        if self.data_type == Type.FLOAT32 :
            return str(self.value)
        if self.data_type == Type.FLOAT64 :
            return str(self.value)
        if self.data_type == Type.ENUM:
            return str(self.value.get_vx_enum_name())
        if self.data_type == Type.SIZE:
            return str(self.value)
        if self.data_type == Type.DF_IMAGE:
            return str(self.value.get_vx_enum_name())
        if self.data_type == Type.BOOL:
            return "vx_" + str(self.value).lower() + "_e"
