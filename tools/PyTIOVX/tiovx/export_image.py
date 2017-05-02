'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *
import subprocess

class ExportImage (Export) :
    def __init__(self, context) :
        Export.__init__(self, context)
        self.filename_prefix = context.name
        self.filename = context.name + "_img.txt"
        self.filenameJpg = context.name + ".jpg"
        self.file = None

    def getDataColor(self, ref) :
        return "GhostWhite"

    def getTargetColor(self, target) :
        if target == Target.DSP1 :
            return "palegreen"
        if target == Target.DSP2 :
            return "darkturquoise"
        if target == Target.EVE1 :
            return "yellow"
        if target == Target.EVE2 :
            return "gold"
        if target == Target.EVE3 :
            return "orange"
        if target == Target.EVE4 :
            return "goldenrod4"
        if target == Target.MPU_0 :
            return "lightblue"
        if target == Target.MPU_1 :
            return "lightblue"
        if target == Target.IPU1_0 :
            return "grey"
        if target == Target.IPU1_1 :
            return "LightSalmon"
        if target == Target.IPU2_0 :
            return "MediumOrchid"
        if target == Target.IPU2_1 :
            return "MediumOrchid"
        return "white"

    def __str__(self) :
        file = open(self.filename,'r')
        str = 'Export Image [' + self.filename + ' ]\n'
        str += file.read()
        file.close()
        return str

    def outputTarget(self, target) :
        self.file.write('        <TR><TD bgcolor="%s">%s</TD></TR>' % (self.getTargetColor(target), target.name))

    def outputTargetList(self) :
        self.file.write('  ColorScheme [shape=none, margin=0, label=<\n')
        self.file.write('        <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n')
        for target in self.context.target_list :
            self.outputTarget(target)
        self.file.write('        </TABLE>>];\n')
        self.file.write('\n')
        self.file.write('\n')

    def outputData(self, data) :
        self.file.write('  %s [color=%s, style=filled]\n' % (data.name, self.getDataColor(data)))

    def outputDataList(self) :
        self.file.write('\n')
        self.file.write('  /* DATA OBJECTS */\n')
        for ref in self.context.data_list :
            self.outputData(ref)
        self.file.write('\n')

    def outputNode(self, node) :
        self.file.write('  %s [label=\"%s (%s)\", color=%s, style=filled]\n' % (node.name, node.name, node.kernel, self.getTargetColor(node.target)) )

    def outputNodeList(self) :
        self.file.write('\n')
        self.file.write('  /* NODE OBJECTS */\n')
        for ref in self.context.node_list :
            self.outputNode(ref)
        self.file.write('\n')

    def outputNodeConnection(self, node) :
        idx = 0
        for dir in node.param_dir :
            if dir == Direction.INPUT :
                self.file.write('  %s -> %s [taillabel=%d, labeldistance=3]\n' % (node.ref[idx].name, node.name, idx))
            else :
                self.file.write('  %s -> %s [headlabel=%d, labeldistance=3]\n' % (node.name, node.ref[idx].name, idx))
            idx = idx + 1

    def outputNodeConnectionList(self) :
        self.file.write('\n')
        self.file.write('  /* NODE CONNECTIONS */\n')
        for node in self.context.node_list :
            self.outputNodeConnection(node)
        self.file.write('\n')

    def export(self) :
        print ('Generating image from OpenVX context ...')
        print ('dot tool input  file is [%s] ...' % self.filename)
        print ('dot tool output file is [%s] ...' % self.filenameJpg)

        self.file = open(self.filename, 'w')
        self.file.write('digraph %s {\n' % self.context.name)
        self.file.write('\n')
        self.file.write('  label = \"%s\"\n' % self.context.name)
        self.outputTargetList()
        self.outputDataList()
        self.outputNodeList()
        self.outputNodeConnectionList()
        self.file.write('\n')
        self.file.write('}\n')
        self.file.close()

        try :
            command_str = 'dot %s -Tjpg -o%s' % (self.filename, self.filenameJpg)
            print('Executing dot tool command ... [' + command_str + ']')
            subprocess.call(command_str)
            print ('Generating image from OpenVX context ... DONE !!!')
        except FileNotFoundError:
            print('ERROR: \'dot\' tool not found. Make sure \'graphviz\' is installed and \'dot\' command is added to system PATH !!!')
            print('ERROR: Cannot generate .jpg file !!!')

