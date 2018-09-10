*** PyTIOVX ***

'tiovx' is Python module to specify OpenVX graph in compact manner using
predefined class and objects. After specification of OpenVX graph it
can be exported in various formats including C code, JPG image

Installation
============
1. Install Python 3.5.2 or earlier (https://www.python.org/)
   - To confirm "python" and "pip" are in your install path
   - Type 
     # python --version
     # pip --version

2. Install 'dot' tool provided as part of 'graphviz' (http://www.graphviz.org/)
   'dot' is required to generated .JPG file for a OpenVX graph specification
   - To confirm "dot" tool is in your install path, type
     # dot -V
   
3. Install 'tiovx' module by executing below command at folder specified
   Folder: <tiovx install path>/tiovx/tools/PyTIOVX
   Type Command:
   # sudo pip3 install -e .

   Expected output,
     Obtaining file:///<tiovx install path>/tiovx/tools/PyTIOVX
     Installing collected packages: tiovx
       Running setup.py develop for tiovx
     Successfully installed tiovx-0.1

4. Now tiovx python module can be imported and used in your python script 
   by doing below,
   from tiovx import *

Usage
=====
Refer to sample python scripts (*.py) in 
<tiovx install path>/tiovx/tools/sample_use_cases for sample usage of tiovx module
