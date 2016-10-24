'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *
    
class Graph (Reference) :
    def __init__(self, name="default") :
        Reference.__init__(self, Type.GRAPH, name)
        self.ref = []

    def add(self,node) :
        self.ref.append(node)
    
    def __str__(self):
        print_str = Reference.__str__(self) 
        for ref in self.ref :
            print_str = print_str + '\n' + str(ref)
        return print_str

