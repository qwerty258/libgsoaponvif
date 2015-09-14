// testLibXML.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <libxml\parser.h>
#include <Windows.h>

typedef xmlDocPtr(*functionxmlReadMemory)(const char* buffer, int size, const char* URL, const char * encoding, int options);
typedef void(*functionxmlFreeDoc)(xmlDocPtr cur);
typedef void(*functionxmlMemoryDump)(void);

functionxmlReadMemory   xmlReadMemoryFunction;
functionxmlFreeDoc      xmlFreeDocFunction;
functionxmlMemoryDump   xmlMemoryDumpFunction;
functionxmlMemoryDump   xmlCleanupParserFunction;

void handleError(TCHAR* message, TCHAR* sourceFileName, int sourceFileLine)
{
    TCHAR buffer[2048];
#ifdef UNICODE
    _snwprintf_s(buffer, 2048, _TRUNCATE, _T("message: %s, error code: %d, file: %s, line: %d"), message, GetLastError(), sourceFileName, sourceFileLine);
#else
    _snprintf_s(buffer, 2048, _TRUNCATE, _T("message: %s, error code: %d, file: %s, line: %d"), message, GetLastError(), sourceFileName, sourceFileLine);
#endif // UNICODE
    MessageBox(NULL, buffer, _T("Error"), MB_OK);
}

int _tmain(int argc, _TCHAR* argv[])
{
    HMODULE hDLL = LoadLibrary(_T("libxml2-2.dll"));
    if(NULL == hDLL)
    {
        handleError(_T("LoadLibrary"), _T(__FILE__), __LINE__);
        return -1;
    }

    xmlReadMemoryFunction = (functionxmlReadMemory)GetProcAddress(hDLL, "xmlReadMemory");
    if(NULL == xmlReadMemoryFunction)
    {
        handleError(_T("GetProcAddress"), _T(__FILE__), __LINE__);
        return -1;
    }

    xmlFreeDocFunction = (functionxmlFreeDoc)GetProcAddress(hDLL, "xmlFreeDoc");
    if(NULL == xmlFreeDocFunction)
    {
        handleError(_T("GetProcAddress"), _T(__FILE__), __LINE__);
        return -1;
    }

    xmlMemoryDumpFunction = (functionxmlMemoryDump)GetProcAddress(hDLL, "xmlMemoryDump");
    if(NULL == xmlMemoryDumpFunction)
    {
        handleError(_T("GetProcAddress"), _T(__FILE__), __LINE__);
        return -1;
    }

    xmlCleanupParserFunction = (functionxmlMemoryDump)GetProcAddress(hDLL, "xmlCleanupParser");
    if(NULL == xmlMemoryDumpFunction)
    {
        handleError(_T("GetProcAddress"), _T(__FILE__), __LINE__);
        return -1;
    }

    char* testXML = (char*)malloc(2048);
    memset(testXML, 0x0, 2048);
    _snprintf_s(testXML, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"utf-8\"?><SOAP-ENV:Envelope xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\"><SOAP-ENV:Header><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action><wsa:MessageID>uuid:e32e6863-ea5e-4ee4-997e-69539d1ff2cc</wsa:MessageID><wsa:RelatesTo>uuid:759a9a95-610f-40a9-bf26-fe2c2836ed50</wsa:RelatesTo><wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:ProbeMatches><wsdd:ProbeMatch><wsa:EndpointReference><wsa:Address>urn:uuid:4fe963b6-e06a-409b-8000-3C970E326186</wsa:Address></wsa:EndpointReference><wsdd:Types>tdn:NetworkVideoTransmitter</wsdd:Types><wsdd:Scopes>onvif://www.onvif.org/type/NetworkVideoTransmitter</wsdd:Scopes><wsdd:XAddrs>http://192.168.10.183:8080/onvif/device_service</wsdd:XAddrs><wsdd:MetadataVersion>10</wsdd:MetadataVersion></wsdd:ProbeMatch></wsdd:ProbeMatches></SOAP-ENV:Body></SOAP-ENV:Envelope>");

    for(size_t i = 0; i < 1000; i++)
    {
        xmlDocPtr pxmlDocPtr = xmlReadMemoryFunction(testXML, 2048, NULL, NULL, XML_PARSE_RECOVER);

        xmlFreeDocFunction(pxmlDocPtr);
    }

    xmlCleanupParserFunction();

    xmlMemoryDumpFunction();

    free(testXML);
    testXML = NULL;

    if(0 == FreeLibrary(hDLL))
    {
        handleError(_T("FreeLibrary"), _T(__FILE__), __LINE__);
        return -1;
    }

    return 0;
}

