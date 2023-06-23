#!/bin/bash

pushd onvif-wsdl
# gsoap 2.8.124-2 on debian, do not needs to download gsoap source code.
wsdl2h -c -O4 -I ./ -t /usr/share/gsoap/WS/typemap.dat -o gsoaponvif.h \
    devicemgmt.wsdl

soapcpp2 -2 -c -C -x -I /usr/share/gsoap/import gsoaponvif.h

mv gsoaponvif.h soapC.c soapClient.c soapClientLib.c soapH.h soapStub.h ../generated_code/

popd

