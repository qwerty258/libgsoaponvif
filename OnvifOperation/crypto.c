#include "crypto.h"
#include "errorHandling.h"
#include <tchar.h>
#include <openssl\rand.h>
#include <openssl\sha.h>
#include <openssl\evp.h>
#include <openssl\bio.h>
#include <openssl\buffer.h>
#include <stdio.h>
#include <Windows.h>

int generateEncrytedAuthorizationInformation(unsigned char* bufferForNonce, int nonceSize, unsigned char* encrytedPassword, char* password, int passwordSize, char* timeBuffer, int timeBufferSize)
{
    if(NULL == bufferForNonce || 0 == nonceSize || NULL == encrytedPassword || NULL == password || 0 == passwordSize || NULL == timeBuffer || 0 == timeBufferSize)
    {
        return -1;
    }

    int result = RAND_bytes(bufferForNonce, nonceSize);
    if(0 == result)
    {
        return -1;
    }

    SYSTEMTIME* systemTime = (SYSTEMTIME*)malloc(sizeof(SYSTEMTIME));
    if(NULL == systemTime)
    {
        handleError(_T("malloc"), _T(__FILE__), __LINE__);
        return -1;
    }

    GetSystemTime(systemTime);

    result = sprintf_s(timeBuffer, timeBufferSize, "%04d-%02d-%02dT%02d:%02d:%02dZ", systemTime->wYear, systemTime->wMonth, systemTime->wDay, systemTime->wHour, systemTime->wMinute, systemTime->wSecond);
    if(0 > result)
    {
        handleError(_T("sprintf_s"), _T(__FILE__), __LINE__);
        return -1;
    }

    int bufferSize = nonceSize + passwordSize + strnlen(timeBuffer, timeBufferSize) + 10;
    char* nonceTimePassword = (char*)malloc(bufferSize);
    if(NULL == nonceTimePassword)
    {
        handleError(_T("malloc"), _T(__FILE__), __LINE__);
        return -1;
    }

    char* pCurrentPosition = nonceTimePassword;

    errno_t error;
    error = memcpy_s(pCurrentPosition, bufferSize, bufferForNonce, nonceSize);
    if(0 != error)
    {
        handleError(_T("memcpy_s"), _T(__FILE__), __LINE__);
        return -1;
    }
    pCurrentPosition += nonceSize;

    error = memcpy_s(pCurrentPosition, bufferSize - (pCurrentPosition - nonceTimePassword), timeBuffer, strnlen(timeBuffer, timeBufferSize));
    if(0 != error)
    {
        handleError(_T("memcpy_s"), _T(__FILE__), __LINE__);
        return -1;
    }
    pCurrentPosition += strnlen(timeBuffer, timeBufferSize);

    error = memcpy_s(pCurrentPosition, bufferSize - (pCurrentPosition - nonceTimePassword), password, passwordSize);
    if(0 != error)
    {
        handleError(_T("memcpy_s"), _T(__FILE__), __LINE__);
        return -1;
    }

    SHA_CTX* shaContext = (SHA_CTX*)malloc(sizeof(SHA_CTX));
    if(NULL == shaContext)
    {
        handleError(_T("malloc"), _T(__FILE__), __LINE__);
        return -1;
    }

    result = SHA1_Init(shaContext);
    if(1 != result)
    {
        handleError(_T("SHA1_Init"), _T(__FILE__), __LINE__);
        return -1;
    }

    result = SHA1_Update(shaContext, nonceTimePassword, nonceSize + strnlen(timeBuffer, timeBufferSize) + passwordSize);
    if(1 != result)
    {
        handleError(_T("SHA1_Update"), _T(__FILE__), __LINE__);
        return -1;
    }

    result = SHA1_Final(encrytedPassword, shaContext);
    if(1 != result)
    {
        handleError(_T("SHA1_Final"), _T(__FILE__), __LINE__);
        return -1;
    }

    if(NULL != shaContext)
    {
        free(shaContext);
        shaContext = NULL;
    }

    if(NULL != nonceTimePassword)
    {
        free(nonceTimePassword);
        nonceTimePassword = NULL;
    }

    if(NULL != systemTime)
    {
        free(systemTime);
        systemTime = NULL;
    }

    return 0;
}

base64encodeContext* getBase64encodeContext(unsigned char* rawData, int rawDataSize)
{
    if(NULL == rawData || 0 == rawData)
    {
        return NULL;
    }

    base64encodeContext* pbase64encodeContext = (base64encodeContext*)malloc(sizeof(base64encodeContext));
    if(NULL == pbase64encodeContext)
    {
        handleError(_T("malloc"), _T(__FILE__), __LINE__);
        return pbase64encodeContext;
    }

    pbase64encodeContext->b64 = BIO_new(BIO_f_base64());
    if(NULL == pbase64encodeContext->b64)
    {
        freeBase64encodeContext(&pbase64encodeContext);
        return pbase64encodeContext;
    }

    pbase64encodeContext->bmem = BIO_new(BIO_s_mem());
    if(NULL == pbase64encodeContext->bmem)
    {
        freeBase64encodeContext(&pbase64encodeContext);
        return pbase64encodeContext;
    }

    BIO* b64Temp = BIO_push((BIO*)pbase64encodeContext->b64, (BIO*)pbase64encodeContext->bmem);
    if(NULL == b64Temp)
    {
        freeBase64encodeContext(&pbase64encodeContext);
        return pbase64encodeContext;
    }
    pbase64encodeContext->b64 = b64Temp;

    int result = BIO_write((BIO*)pbase64encodeContext->b64, rawData, rawDataSize);
    if(0 >= result)
    {
        freeBase64encodeContext(&pbase64encodeContext);
        return pbase64encodeContext;
    }

    BIO_get_mem_ptr((BIO*)pbase64encodeContext->b64, &(pbase64encodeContext->bptr));

    return pbase64encodeContext;
}

void freeBase64encodeContext(base64encodeContext** base64Context)
{
    if(NULL == base64Context || NULL == (*base64Context))
    {
        return;
    }

    base64encodeContext* pbase64encodeContext = (base64encodeContext*)(*base64Context);

    BIO_free_all((BIO*)pbase64encodeContext->b64);

    free(pbase64encodeContext);

    (*base64Context) = NULL;
}

size_t getBase64encodeResultSize(base64encodeContext* base64Context)
{
    if(NULL == base64Context)
    {
        return -1;
    }

    return ((BUF_MEM*)base64Context->bptr)->length;
}

int getBase64encodeResult(base64encodeContext* base64Context, char* buffer, int bufferSize)
{
    if(NULL == base64Context || NULL == buffer)
    {
        return -1;
    }

    errno_t error = memcpy_s(buffer, bufferSize, ((BUF_MEM*)base64Context->bptr)->data, ((BUF_MEM*)base64Context->bptr)->length);
    if(0 != error)
    {
        handleError(_T("memcpy_s"), _T(__FILE__), __LINE__);
        return -1;
    }
}