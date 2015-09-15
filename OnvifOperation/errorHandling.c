#include "errorHandling.h"
#include <tchar.h>
#include <Windows.h>

void handleError(void* message, void* sourceFileName, int sourceFileLine)
{
    TCHAR buffer[2048];
#ifdef UNICODE
    _snwprintf_s(buffer, 2048, _TRUNCATE, _T("message: %s, error code: %d, file: %s, line: %d"), message, GetLastError(), sourceFileName, sourceFileLine);
#else
    _snprintf_s(buffer, 2048, _TRUNCATE, _T("message: %s, error code: %d, file: %s, line: %d"), message, GetLastError(), sourceFileName, sourceFileLine);
#endif // UNICODE
    MessageBox(NULL, buffer, _T("Error"), MB_OK);
}
