#ifdef TRACES

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

void
_to_log(const char* fmt, ...)
{
    DWORD last_error;
    va_list args;
    char buffer[512];
    int offset = 0;
    int n;

    last_error = GetLastError();

    offset += sprintf_s(buffer, sizeof(buffer), "[%08x] ", (unsigned) GetCurrentThreadId());
	size_t count = sizeof(buffer)-offset-2;

    va_start(args, fmt);
	n = _vsnprintf_s(buffer + offset, count, _TRUNCATE, fmt, args);
    va_end(args);
    if(n > 0) {
		offset += n;
		buffer[offset++] = '\n';
		buffer[offset++] = '\0';
	#ifdef DEBUG
		OutputDebugStringA(buffer); //outputs to debuger's window
	#else
		printf_s("%s", buffer); // outputs to normal window
	#endif
	}
    SetLastError(last_error);
}

#endif  /* TRACES */
