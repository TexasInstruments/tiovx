'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Object Array object (OpenVX equivalent = vx_object_array)
#
# <b> NOT SUPPORTED in tool </b>
#
# \ingroup DATA
class ObjectArray (Reference) :
    def __init__(self, type, count, name="default") :
        Reference.__init__(self, Type.OBJECTARRAY, name)
        self.item_type = data_type
        self.count = count

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.item_type.name + ':' + str(self.count) + ' ]'

