'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Lut object (OpenVX equivalent = vx_lut)
#
# \par Example Usage: Create a LUT object of U8 of size 10
#
# \code
# from tiovx import *
#
# my_lut1 = Lut(Type.UINT8, 10, name="mylut")
# my_lut2 = Lut(Type.UINT8, 10)
# \endcode
#
# \ingroup DATA
class Lut (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateLUT for more details about the parameters
    #
    # \param data_type [in] Data type. tiovx::enums::Type
    # \param count [in] Sizeof LUT
    # \param name [in] [optional] Name of the object
    def __init__(self, data_type, count, name="default") :
        Reference.__init__(self, Type.LUT, name)
        self.data_type = data_type
        self.count = count

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ', ' + str(self.count) + ' ]'

