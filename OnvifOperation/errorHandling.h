#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // pointer of message and sourceFileName must be TCHAR*
    void handleError(void* message, void* sourceFileName, int sourceFileLine);

#ifdef __cplusplus
}
#endif // __cplusplus
