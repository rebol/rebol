#pragma once
#undef IS_ERROR
#include <windows.h>
#include <wincodec.h>    // for WIC
#include <wincodecsdk.h> // for saving image

#ifdef __cplusplus
#define CODECS_API extern "C"

extern "C" {
#include "sys-utils.h"
//#include "reb-types.h"
#include "reb-codec.h"
}
#else
#define CODECS_API
//#include "sys-utils.h"
//#include "reb-types.h"
//#include "reb-codec.h"

#endif



CODECS_API int  codecs_init();
CODECS_API void codecs_fini();
CODECS_API int DecodeImageFromFile(PCWSTR *uri, UINT frame, REBCDI *codi);
CODECS_API int EncodeImageToFile(PCWSTR *uri, REBCDI *codi);

CODECS_API int LoadBitmapFromFile(
	PCWSTR uri,
    IWICFormatConverter **ppBitmap
);
CODECS_API int SaveBitmap(IWICBitmap *bitmap, PCWSTR *uri, INT type);

