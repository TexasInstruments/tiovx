'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Scalar object (OpenVX equivalent = vx_scalar)
#
# \par Example Usage: Create a Scalar object
#
# \code
# from tiovx import *
#
# my_scalar1 = Scalar(Type.UINT8, 8, name="myscalar")
# my_scalar2 = Scalar(Type.FLOAT32, 0.8)
# my_scalar3 = Scalar(Type.CHAR, 'c')
# \endcode
#
# \ingroup DATA
class Scalar (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateScalar for more details about the parameters
    #
    # \param data_type [in] Data type. tiovx::enums::Type
    # \param value [in] Value of type 'data_type'
    # \param name [in] [optional] Name of the object
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
            assert ( self.value > -127 and self.value <= 128)
            return
        if self.data_type == Type.UINT8 :
            assert ( self.value >= 0 and self.value <= 255)
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
