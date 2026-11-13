
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dlfcn.h>

CFURLRef urlFromCString(const char* cString);

#ifdef TRACES
	#define TRACE NSLog
#else
	static void _no_log(NSString *format, ...){}
	#define TRACE _no_log
#endif

#define TRACE_MSG(msg)                TRACE(@"" msg)
#define TRACE_FAILED(msg, err)        TRACE(@"FAILED: " msg " [%i]", (err))
#define TRACE_PTR(msg, ptr)           TRACE(@"" msg " [%0lX]", ptr)

#define ASSERT_NOT_NULL(v, e, msg) if(!v) {TRACE_FAILED(msg, e); error = e; break;}

#define SAFE_CF_RELEASE(obj) if(obj != NULL) {CFRelease(obj); obj = NULL;}

#endif
