'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

import subprocess
import os

for file in os.listdir("."):
    if file.endswith(".py"):
        if  file != __file__ :
            print('### Executing test case [%s]' % file)
            subprocess.call('python ' + file)
            print('### Executing test case [%s] ... DONE !!!' % file)
            print('')