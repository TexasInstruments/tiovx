mkdir api
copy *.gif api\.
doxygen api.cfg 2> doxy_warnings.txt
copy api\*.chm ..\
rmdir /s /q api
