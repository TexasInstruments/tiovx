'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Array (Reference) :
    def __init__(self, item_type, capacity, name="default") :
        Reference.__init__(self, Type.ARRAY, name)
        self.item_type = item_type
        self.capacity = capacity

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.item_type.name + ', ' + str(self.capacity) + ' ]'

