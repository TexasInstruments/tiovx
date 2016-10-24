'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Reference :
    id = 0

    def make_default_name(type) :
        name = type.name.lower() + '_' + str(Reference.id)
        Reference.id = Reference.id + 1
        return name

    def setName(self, name) :
        if ( name == "default" ) :
            self.name = Reference.make_default_name(self.type)
        else :
            self.name = name    

    def __init__(self, type, name) :
        self.type = type 
        self.setName(name)
        
    def __str__(self):
        return self.name + ' ( ' + self.type.name + ' )'

