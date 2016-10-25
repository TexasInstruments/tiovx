'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Remap (Reference) :
    def __init__(self, src_width, src_height, dst_width, dst_height, name="default") :
        Reference.__init__(self, Type.REMAN, name)
        self.src_width = src_width
        self.src_height = src_height
        self.dst_width = dst_width
        self.dst_height = dst_height

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.src_width) + 'x' + str(self.src_height) + ' ] ' + ' [ ' + str(self.dst_width) + 'x' + str(self.dst_height) + ' ]'

