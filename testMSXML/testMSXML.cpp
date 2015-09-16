// testMSXML.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <objbase.h>
#include <comutil.h>
#include <MsXml6.h>

IXMLDOMNode* findNode(IXMLDOMNode* pIXMLDOMNode, char* pSzElementName)
{
    IXMLDOMNode* pIXMLDOMNodeTemp = NULL;
    IXMLDOMNodeList* pIXMLDOMNodeList = NULL;
    long length = 0;
    long releaseCount = 0;
    HRESULT hResult = 0;
    BSTR nodeName = NULL;
    char* szNodeName = NULL;

    hResult = pIXMLDOMNode->get_childNodes(&pIXMLDOMNodeList);
    if(FAILED(hResult))
    {
        goto findNodeCleanUp;
    }

    hResult = pIXMLDOMNodeList->get_length(&length);
    if(FAILED(hResult))
    {
        goto findNodeCleanUp;
    }

    for(long i = 0; i < length; i++)
    {
        pIXMLDOMNodeList->get_item(i, &pIXMLDOMNodeTemp);
        pIXMLDOMNodeTemp->get_nodeName(&nodeName);
        szNodeName = _com_util::ConvertBSTRToString(nodeName);

        SysFreeString(nodeName);
        nodeName = NULL;

        if(NULL != strstr(szNodeName, pSzElementName))
        {
            delete[] szNodeName;
            goto findNodeCleanUp;
        }

        delete[] szNodeName;

        if(NULL != pIXMLDOMNodeTemp)
        {
            do
            {
                releaseCount = pIXMLDOMNodeTemp->Release();
            } while(releaseCount > 0);
            pIXMLDOMNodeTemp = NULL;
        }
    }

findNodeCleanUp:

    if(NULL != pIXMLDOMNodeList)
    {
        do
        {
            releaseCount = pIXMLDOMNodeList->Release();
        } while(releaseCount > 0);
        pIXMLDOMNodeList = NULL;
    }

    return pIXMLDOMNodeTemp;
}

int _tmain(int argc, _TCHAR* argv[])
{
    char* testXML = (char*)malloc(2048);
    memset(testXML, 0x0, 2048);
    _snprintf_s(testXML, 2048, _TRUNCATE, "<?xml version = \"1.0\" encoding = \"utf-8\"?><SOAP-ENV:Envelope xmlns:tdn = \"http://www.onvif.org/ver10/network/wsdl\" xmlns:wsa = \"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd = \"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:SOAP-ENV = \"http://www.w3.org/2003/05/soap-envelope\"><SOAP-ENV:Header><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action><wsa:MessageID>uuid:e32e6863-ea5e-4ee4-997e-69539d1ff2cc</wsa:MessageID><wsa:RelatesTo>uuid:759a9a95-610f-40a9-bf26-fe2c2836ed50</wsa:RelatesTo><wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:ProbeMatches><wsdd:ProbeMatch><wsa:EndpointReference><wsa:Address>urn:uuid:4fe963b6-e06a-409b-8000-3C970E326186</wsa:Address></wsa:EndpointReference><wsdd:Types>tdn:NetworkVideoTransmitter</wsdd:Types><wsdd:Scopes>onvif://www.onvif.org/type/NetworkVideoTransmitter</wsdd:Scopes><wsdd:XAddrs>http://192.168.10.183:8080/onvif/device_service</wsdd:XAddrs><wsdd:MetadataVersion>10</wsdd:MetadataVersion></wsdd:ProbeMatch></wsdd:ProbeMatches></SOAP-ENV:Body></SOAP-ENV:Envelope>");

    IXMLDOMDocument* pIXMLDOMDocument = NULL;
    BSTR bstrXMLInMemory = NULL;
    BSTR bstrError = NULL;
    BSTR bstrURI = NULL;
    VARIANT_BOOL varStatus;
    IXMLDOMParseError* pIXMLDOMParseError = NULL;
    IXMLDOMNode* pIXMLDOMNodeTemp = NULL;
    IXMLDOMNode* pIXMLDOMNodeFound = NULL;
    long releaseResult;




    HRESULT hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }


    hResult = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIXMLDOMDocument));
    if(FAILED(hResult))
    {
        goto CleanUp;
    }

    hResult = pIXMLDOMDocument->put_async(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }

    hResult = pIXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }

    hResult = pIXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }

    bstrXMLInMemory = _com_util::ConvertStringToBSTR(testXML);
    hResult = pIXMLDOMDocument->loadXML(bstrXMLInMemory, &varStatus);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }
    if(varStatus != VARIANT_TRUE)
    {
        hResult = pIXMLDOMDocument->get_parseError(&pIXMLDOMParseError);
        if(FAILED(hResult))
        {
            goto CleanUp;
        }

        hResult = pIXMLDOMParseError->get_reason(&bstrError);
        if(FAILED(hResult))
        {
            goto CleanUp;
        }

        _tprintf_s(_T("%s\n"), bstrError);
    }

    pIXMLDOMNodeFound = findNode(pIXMLDOMDocument, "Envelope");
    if(NULL == pIXMLDOMNodeFound)
    {
        goto CleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "Body");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto CleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "ProbeMatches");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto CleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "ProbeMatch");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto CleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "XAddrs");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto CleanUp;
    }

    hResult = pIXMLDOMNodeFound->get_text(&bstrURI);
    if(FAILED(hResult))
    {
        goto CleanUp;
    }

    _tprintf_s(_T("%s\n"), bstrURI);

CleanUp:








    if(NULL != pIXMLDOMNodeFound)
    {
        do
        {
            releaseResult = pIXMLDOMNodeFound->Release();
        } while(releaseResult > 0);
        pIXMLDOMNodeFound = NULL;
    }

    //if(NULL != pIXMLDOMNode)
    //{
    //    do
    //    {
    //        releaseResult = pIXMLDOMNode->Release();
    //    } while(releaseResult > 0);
    //    pIXMLDOMNode = NULL;
    //}

    if(NULL != pIXMLDOMParseError)
    {
        do
        {
            releaseResult = pIXMLDOMParseError->Release();
        } while(releaseResult > 0);
        pIXMLDOMParseError = NULL;
    }

    if(NULL != pIXMLDOMDocument)
    {
        do
        {
            releaseResult = pIXMLDOMDocument->Release();
        } while(releaseResult > 0);
        pIXMLDOMDocument = NULL;
    }

    SysFreeString(bstrError);
    SysFreeString(bstrXMLInMemory);
    SysFreeString(bstrURI);

    CoUninitialize();

    free(testXML);
    testXML = NULL;

    system("pause");

    return 0;
}

