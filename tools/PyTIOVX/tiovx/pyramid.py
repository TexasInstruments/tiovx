'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Pyramid (Reference) :
    def __init__(self, levels, scale, width, height, df_format, name="default") :
        Reference.__init__(self, Type.PYRAMID, name)
        self.num_levels = levels;
        self.width = width;
        self.height = height;
        self.scale = scale;
        self.format = format;

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.format.name + ':' + str(self.scale) + ', ' + str(self.width) + ', ' + str(self.height) + ' ]'
