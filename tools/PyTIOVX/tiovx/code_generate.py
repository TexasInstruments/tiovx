'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *
    
class CodeGenerate :
    def __init__(self, filename) :
        self.indent_level = 0
        self.indent = "    "
        self.filename = filename
        self.open()

    def open(self) :
        self.file = open(self.filename,'w')
        self.write_header()

    def close(self) :
        self.write_newline()
        self.file.close()

    def write_header(self) :
        self.write_line('/*')
        self.write_line(' * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/')
        self.write_line(' * ALL RIGHTS RESERVED')
        self.write_line(' */')
        self.write_newline()

    def write_comment_line(self, text_line) :
        self.write_line('/* %s */' % text_line)

    def write_line(self, text_line) :
        for i in range(0, self.indent_level) :
            self.file.write(self.indent)
        self.file.write(text_line)
        self.file.write('\n')
    
    def write_open_brace(self) :
        self.write_line('{')
        self.indent_level = self.indent_level+1
        
    def write_close_brace(self, text="") :
        self.indent_level = self.indent_level-1
        self.write_line('}%s' % text)

    def write_include(self, include_file_name) :
        self.write_line('#include <%s>' % include_file_name)

    def write_ifndef_define(self, text) :
        self.write_line('#ifndef %s' % text)
        self.write_line('#define %s' % text)
        self.write_newline()
    
    def write_endif(self, text) :
        self.write_line('#endif /* %s */' % text)
        self.write_newline()

    def write_newline(self) :
        self.write_line('')


        
