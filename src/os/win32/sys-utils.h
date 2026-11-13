//#define TRACES // to enable trace output


#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER > 1200)
#define _no_log(...)     do { } while(0)
#else
static void _no_log(const char* fmt, ...) {}
#endif

#ifdef TRACES

    void _to_log(const char* fmt, ...);
    #define TRACE    _to_log
#else
    #define TRACE    _no_log
#endif

#define TRACE_ERR_(msg, err)          TRACE("FAILED: " msg " [%lu]", (err))
#define TRACE_ERR(msg)                TRACE("FAILED: " msg " [%lu]", GetLastError())
#define TRACE_FAILED_HR(msg)          TRACE("FAILED: " msg " [%lx]", hr)
#define TRACE_HR_(msg, hr)            TRACE(msg " [%lx]", (hr))
#define TRACE_HR(msg)                 TRACE(msg " [%lx]", hr)
#define TRACE_PTR(msg, ptr)           TRACE(msg " [%0lX]", ptr)

#define ASSERT_HR(msg) if(FAILED(hr)) {TRACE_FAILED_HR(msg); break;}

#define RELEASE(obj) if(obj) {obj->Release(); obj = NULL;}
