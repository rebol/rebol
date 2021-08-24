#pragma once

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dlfcn.h>

#define CODECS_API
//#define CODECS_API extern "C"

//extern "C" {
//#include "sys-utils.h"
//#include "reb-types.h"
//}
#else
#define CODECS_API
#endif
#include "reb-codec.h"

CODECS_API int  codecs_init();
CODECS_API void codecs_fini();
CODECS_API int DecodeImageFromFile(const char *uri, unsigned int frame, REBCDI *codi);
CODECS_API int EncodeImageToFile(const char *uri, REBCDI *codi);
