'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Threshold (Reference) :
    def __init__(self, thr_type, data_type, name="default") :
        Reference.__init__(self, Type.THRESHOLD, name)
        self.thr_type = thr_type
        self.data_type = data_type

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.thr_type.name + ', ' + self.data_type.nane + ' ] '

