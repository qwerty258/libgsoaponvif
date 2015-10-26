COPY ".\x64\Release\OnvifOperation.dll"	".\OnvifOperationSDK\x64\bin"
COPY ".\x64\Release\OnvifOperation.lib"	".\OnvifOperationSDK\x64\lib"
COPY ".\x64\Release\test.exe"	".\OnvifOperationSDK\x64\bin"

COPY ".\x86\Release\OnvifOperation.dll"	".\OnvifOperationSDK\x86\bin"
COPY ".\x86\Release\OnvifOperation.lib"	".\OnvifOperationSDK\x86\lib"
COPY ".\x86\Release\test.exe"	".\OnvifOperationSDK\x86\bin"

COPY ".\OnvifOperation接口说明.docx"	".\OnvifOperationSDK\doc"

COPY ".\test\test.cpp"	."\OnvifOperationSDK\example"
