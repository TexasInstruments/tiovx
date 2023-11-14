mkdir ..\..\..\docs\user_guide
copy *.gif ..\..\..\docs\user_guide
doxygen user_guide.cfg 2> doxy_warnings.txt
