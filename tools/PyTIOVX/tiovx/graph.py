'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Graph object (OpenVX equivalent = vx_graph)
#
#
# \par Example Usage: Create a graph and add nodes to it
#
# \code
#
# from tiovx import *
#
# my_context = Context("my_context_name")
#
# my_graph = Graph("mygraph")
#
# my_context.add(my_graph)
#
# my_graph.add(my_node)
# my_graph.add(my_node)
#
# \endcode
#
# \ingroup FRAMEWORK
class Graph (Reference) :
    ## Constructor used to create this object
    #
    # \param name [in] Name to assign to this context
    def __init__(self, name="default") :
        Reference.__init__(self, Type.GRAPH, name)
        self.ref = []

    ## Add node object to graph
    #
    # \param ref [in] Object of type Node
    def add(self,node) :
        self.ref.append(node)

    def __str__(self):
        print_str = Reference.__str__(self)
        for ref in self.ref :
            print_str = print_str + '\n' + str(ref)
        return print_str

