How to generate the API Guide .CHM file
***************************************

Installation
============
- Install doxygen for Windows from the below link.
  http://www.stack.nl/~dimitri/doxygen/download.html#latestsrc

- Current CFG works with only 1.6.1 Doxygen version

- In case doxygen.exe does not appear in your system PATH.
  Add it manually via windows control panel.

- Install HTML Help Workshop for Windows from the below link.
  http://www.microsoft.com/downloads/details.aspx?FamilyID=00535334-c8a6-452f-9aa0-d597d16580cc&displaylang=en

  This is used to convert doxygen output to .CHM file.

- Edit "api.cfg", goto below line and change path
  according to where HTML Help workshop was installed
  in your system.

  HHC_LOCATION           = "c:\Program Files\HTML Help Workshop\hhc.exe "


Generating the documention
==========================
- Run the batch file "build_doc.bat"

- This will generate a .CHM at below path
  \tiovx\docs\*.CHM

- It will delete all temporary generated files.

