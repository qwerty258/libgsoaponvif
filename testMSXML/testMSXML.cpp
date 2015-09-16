// testMSXML.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <objbase.h>
#include <comutil.h>
#include <MsXml6.h>

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)\

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
    HRESULT hr = S_OK;
    BSTR bstr = SysAllocString(wszValue);
    CHK_ALLOC(bstr);

    V_VT(&Variant) = VT_BSTR;
    V_BSTR(&Variant) = bstr;

CleanUp:
    return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc)
{
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
    if(SUCCEEDED(hr))
    {
        // these methods should not fail so don't inspect result
        (*ppDoc)->put_async(VARIANT_FALSE);
        (*ppDoc)->put_validateOnParse(VARIANT_FALSE);
        (*ppDoc)->put_resolveExternals(VARIANT_FALSE);
    }
    return hr;
}

void loadDOMRaw(char* testXML)
{
    HRESULT hr = S_OK;
    IXMLDOMDocument *pXMLDom = NULL;
    IXMLDOMParseError *pXMLErr = NULL;

    IXMLDOMNodeList* pIXMLDOMNodeList = NULL;
    IXMLDOMNodeList* pIXMLDOMNodeListOfEnvelope = NULL;

    IXMLDOMNode* pIXMLDOMNode = NULL;
    IXMLDOMNode* pIXMLDOMNodeOfEnvelope = NULL;

    BSTR nodeName = NULL;
    BSTR nodeNameOfEnvelope = NULL;
    char* szNodeName;
    char* szNodeNameOfEnvelope;

    long length;
    long lengthOfEnvelope;
    long releaseCount;

    BSTR bstrXML = NULL;
    BSTR bstrErr = NULL;

    BSTR bstrXMLInMemory = NULL;

    VARIANT_BOOL varStatus;
    VARIANT varFileName;
    VariantInit(&varFileName);

    CHK_HR(CreateAndInitDOM(&pXMLDom));

    // XML file name to load
    // CHK_HR(VariantFromString(L"stocks.xml", varFileName));
    // CHK_HR(pXMLDom->load(varFileName, &varStatus));

    bstrXMLInMemory = _com_util::ConvertStringToBSTR(testXML);
    CHK_HR(pXMLDom->loadXML(bstrXMLInMemory, &varStatus));
    if(varStatus == VARIANT_TRUE)
    {
        CHK_HR(pXMLDom->get_xml(&bstrXML));
        printf("XML DOM loaded from stocks.xml:\n%S\n", bstrXML);
        pXMLDom->get_childNodes(&pIXMLDOMNodeList);

        pIXMLDOMNodeList->get_length(&length);
        for(long i = 0; i < length; i++)
        {
            pIXMLDOMNodeList->get_item(i, &pIXMLDOMNode);
            pIXMLDOMNode->get_nodeName(&nodeName);
            szNodeName = _com_util::ConvertBSTRToString(nodeName);
            printf_s("%s\n", szNodeName);

            if(NULL != strstr(szNodeName, "Envelope"))
            {
                pIXMLDOMNode->get_childNodes(&pIXMLDOMNodeListOfEnvelope);

                pIXMLDOMNodeListOfEnvelope->get_length(&lengthOfEnvelope);
                for(long i = 0; i < lengthOfEnvelope; i++)
                {
                    pIXMLDOMNodeListOfEnvelope->get_item(i, &pIXMLDOMNodeOfEnvelope);
                    pIXMLDOMNodeOfEnvelope->get_nodeName(&nodeNameOfEnvelope);
                    szNodeNameOfEnvelope = _com_util::ConvertBSTRToString(nodeNameOfEnvelope);
                    printf_s("%s\n", szNodeNameOfEnvelope);

                    delete[] szNodeNameOfEnvelope;

                    do
                    {
                        releaseCount = pIXMLDOMNodeOfEnvelope->Release();
                    } while(releaseCount > 0);
                    pIXMLDOMNodeOfEnvelope = NULL;

                    SysFreeString(nodeNameOfEnvelope);
                    nodeNameOfEnvelope = NULL;
                }

                do
                {
                    releaseCount = pIXMLDOMNodeListOfEnvelope->Release();
                } while(releaseCount > 0);

            }

            delete[] szNodeName;

            do
            {
                releaseCount = pIXMLDOMNode->Release();
            } while(releaseCount > 0);
            pIXMLDOMNode = NULL;

            SysFreeString(nodeName);
            nodeName = NULL;
        }

        do
        {
            releaseCount = pIXMLDOMNodeList->Release();
        } while(releaseCount > 0);
    }
    else
    {
        // Failed to load xml, get last parsing error
        CHK_HR(pXMLDom->get_parseError(&pXMLErr));
        CHK_HR(pXMLErr->get_reason(&bstrErr));
        printf("Failed to load DOM from stocks.xml. %S\n", bstrErr);
    }

CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pXMLErr);
    SysFreeString(bstrXML);
    SysFreeString(bstrErr);
    SysFreeString(bstrXMLInMemory);
    VariantClear(&varFileName);
}


int _tmain(int argc, _TCHAR* argv[])
{
    char* testXML = (char*)malloc(2048);
    memset(testXML, 0x0, 2048);
    _snprintf_s(testXML, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"utf-8\"?><SOAP-ENV:Envelope xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\"><SOAP-ENV:Header><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action><wsa:MessageID>uuid:e32e6863-ea5e-4ee4-997e-69539d1ff2cc</wsa:MessageID><wsa:RelatesTo>uuid:759a9a95-610f-40a9-bf26-fe2c2836ed50</wsa:RelatesTo><wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:ProbeMatches><wsdd:ProbeMatch><wsa:EndpointReference><wsa:Address>urn:uuid:4fe963b6-e06a-409b-8000-3C970E326186</wsa:Address></wsa:EndpointReference><wsdd:Types>tdn:NetworkVideoTransmitter</wsdd:Types><wsdd:Scopes>onvif://www.onvif.org/type/NetworkVideoTransmitter</wsdd:Scopes><wsdd:XAddrs>http://192.168.10.183:8080/onvif/device_service</wsdd:XAddrs><wsdd:MetadataVersion>10</wsdd:MetadataVersion></wsdd:ProbeMatch></wsdd:ProbeMatches></SOAP-ENV:Body></SOAP-ENV:Envelope>");


    HRESULT hr = CoInitialize(NULL);
    if(SUCCEEDED(hr))
    {
        loadDOMRaw(testXML);
        CoUninitialize();
    }

    free(testXML);
    testXML = NULL;

    return 0;
}

