'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *
   
class Context (Reference) :
    def __init__(self, name="default") :
        Reference.__init__(self, Type.CONTEXT, name)
        self.ref = []
        self.node_list = []
        self.data_list = []
        self.graph_list = []
        self.target_list = []
        self.is_target_present = {}
        for target in Target :
            self.is_target_present[target.name] = False

    def isDuplicate(self, ref, list) :
        for l in list :
            if l == ref :
                return True
        return False

    def add(self,ref) :
        if self.isDuplicate(ref, self.ref) == False :
            self.ref.append(ref)
            # add to node, data, graph list
            if( ref.type == Type.GRAPH) :
                self.addGraph(ref)
            elif( ref.type == Type.NODE) :
                self.addNode(ref)
            else :
                self.addData(ref)
    
    def __str__(self):
        print_str = Reference.__str__(self)
        for ref in self.ref :
            print_str = print_str + '\n' + str(ref)
        return print_str

    def addGraph(self, ref) :
        ref.__class__ = Graph
        if self.isDuplicate(ref, self.graph_list) == False :
            self.graph_list.append(ref)
            for node in ref.ref :
                self.addNode(node)

    def addData(self, ref) :
        if self.isDuplicate(ref, self.data_list) == False :
            self.data_list.append(ref)

    def addNode(self, ref) :
        ref.__class__ = Node
        if self.isDuplicate(ref, self.node_list) == False :
            self.node_list.append(ref)
            self.addTarget(ref.target)
            for data in ref.ref :
                self.addData(data)

    def addTarget(self, target) :
        # if not already added, then add it
        if self.is_target_present[target.name] == False :
            self.target_list.append(target)
            self.is_target_present[target.name] = True
