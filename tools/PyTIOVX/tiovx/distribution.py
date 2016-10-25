'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Distribution (Reference) :
    def __init__(self, num_bins, offset, range, name="default") :
        Reference.__init__(self, Type.DISTRIBUTION, name)
        self.num_bins = num_bins
        self.offset = offset
        self.range = range

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.num_bins) + ', ' + str(self.offset) + ', ' + str(self.range) + ' ]'

