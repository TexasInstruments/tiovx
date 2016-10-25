'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Lut (Reference) :
    def __init__(self, data_type, count, name="default") :
        Reference.__init__(self, Type.LUT, name)
        self.data_type = data_type
        self.count = count

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ', ' + str(self.count) + ' ]'

