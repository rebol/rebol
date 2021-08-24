//#define TRACES // to enable trace output


#ifdef TRACES
    #define TRACE    printf
#else
    static void _no_log(const char* fmt, ...) {}
    #define TRACE    _no_log
#endif

//#define TRACE_ERR_(msg, err)          TRACE("FAILED: " msg " [%lu]", (err))
//#define TRACE_ERR(msg)                TRACE("FAILED: " msg " [%lu]", GetLastError())
#define TRACE_FAILED(msg)          TRACE("FAILED: " msg " [%lx]")
//#define TRACE_HR_(msg, hr)            TRACE(msg " [%lx]", (hr))
//#define TRACE_HR(msg)                 TRACE(msg " [%lx]", hr)
#define TRACE_PTR(msg, ptr)           TRACE(msg " [%0lX]", ptr)

#define ASSERT_NOT_NULL(v, e, msg) if(!v) {TRACE_FAILED(msg); error = e; break;}

#define SAFE_CF_RELEASE(obj) if(obj != NULL) {CFRelease(obj); obj = NULL;}
