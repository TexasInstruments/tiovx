#
# Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
# ALL RIGHTS RESERVED
#

from . import *

## Array object (OpenVX equivalent = vx_array)
#
#
# \par Example Usage: Create a array object
#
# \code
#
# from tiovx import *
#
# my_array1 = Array(Type.UINT8, 10, name="myarray")
# my_array2 = Array(Type.UINT32, 10)
# \endcode
#
# \ingroup DATA
class Array (Reference) :
    ## Constructor used to create this object
    #
    # \param item_type [in] Data type tiovx::enums::Type
    # \param capacity [in] Number of elements that can be stored in the Array
    # \param name [in] Name of the array object
    def __init__(self, item_type, capacity, name="default") :
        Reference.__init__(self, Type.ARRAY, name)
        self.item_type = item_type
        self.capacity = capacity

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.item_type.name + ', ' + str(self.capacity) + ' ]'

