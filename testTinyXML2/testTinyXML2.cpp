// testTinyXML2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <tinyxml2.h>

int _tmain(int argc, _TCHAR* argv[])
{
    char* testXML = (char*)malloc(2048);
    memset(testXML, 0x0, 2048);
    _snprintf_s(testXML, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"utf-8\"?><SOAP-ENV:Envelope xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\"><SOAP-ENV:Header><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action><wsa:MessageID>uuid:e32e6863-ea5e-4ee4-997e-69539d1ff2cc</wsa:MessageID><wsa:RelatesTo>uuid:759a9a95-610f-40a9-bf26-fe2c2836ed50</wsa:RelatesTo><wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:ProbeMatches><wsdd:ProbeMatch><wsa:EndpointReference><wsa:Address>urn:uuid:4fe963b6-e06a-409b-8000-3C970E326186</wsa:Address></wsa:EndpointReference><wsdd:Types>tdn:NetworkVideoTransmitter</wsdd:Types><wsdd:Scopes>onvif://www.onvif.org/type/NetworkVideoTransmitter</wsdd:Scopes><wsdd:XAddrs>http://192.168.10.183:8080/onvif/device_service</wsdd:XAddrs><wsdd:MetadataVersion>10</wsdd:MetadataVersion></wsdd:ProbeMatch></wsdd:ProbeMatches></SOAP-ENV:Body></SOAP-ENV:Envelope>");

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError xmlError = doc.Parse(testXML);
    tinyxml2::XMLElement* pXMLElement = doc.FirstChildElement("SOAP-ENV:Envelope");
    pXMLElement = pXMLElement->FirstChildElement("SOAP-ENV:Body");
    pXMLElement = pXMLElement->FirstChildElement("wsdd:ProbeMatches");
    pXMLElement = pXMLElement->FirstChildElement("wsdd:ProbeMatch");
    pXMLElement = pXMLElement->FirstChildElement("wsdd:XAddrs");
    printf("%s\n", pXMLElement->FirstChild()->ToText()->Value());

    if(NULL != testXML)
    {
        free(testXML);
        testXML = NULL;
    }
    return 0;
}

