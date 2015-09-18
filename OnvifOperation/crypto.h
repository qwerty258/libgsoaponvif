#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    typedef struct _base64encodeContext
    {
        void* b64;
        void* bmem;
        void* bptr;
    }base64encodeContext;

    int generateEncrytedAuthorizationInformation(unsigned char* bufferForNonce, int nonceSize, unsigned char* encrytedPassword, char* password, int passwordSize, char* timeBuffer, int timeBufferSize);

    base64encodeContext* getBase64encodeContext(unsigned char* rawData, int rawDataSize);
    size_t getBase64encodeResultSize(base64encodeContext* base64Context);
    int getBase64encodeResult(base64encodeContext* base64Context, char* buffer, int bufferSize);
    void freeBase64encodeContext(base64encodeContext** base64Context);

#ifdef __cplusplus
}
#endif // __cplusplus
