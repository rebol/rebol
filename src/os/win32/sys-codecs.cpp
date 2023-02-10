/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**	Copyright 2019-2023 Oldes
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Title: Windows native codecs functionality
**  Author: Oldes
**  Purpose: To load and save images using system API
**	Related: core/n-image.c
**  Note: So far saves just one frame. For multi-image formats it would be
**        better to write a device (image:// port or something like that)
**
************************************************************************
**  Useful links:
**  https://chromium.googlesource.com/webm/webp-wic-codec
***********************************************************************/

#include "sys-codecs.h"

IWICImagingFactory *pIWICFactory = NULL;
static LARGE_INTEGER zero = { 0 };

CODECS_API int codecs_init()
{
	HRESULT hr = S_OK;
	if (!pIWICFactory)
	{
		//Create Windows Imaging Component ImagingFactory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_PPV_ARGS(&pIWICFactory)
		);
	}
	return hr;
}

CODECS_API void codecs_fini()
{
	if (pIWICFactory) pIWICFactory->Release();
}


CODECS_API int DecodeImageFromFile(PCWSTR *uri, UINT frame, REBCDI *codi)
{
	HRESULT hr = S_OK;
	UINT w, h;
	BYTE *data = NULL;
	WICRect wrect;

	IWICBitmapDecoder     *pDecoder   = NULL;
    IWICBitmapFrameDecode *pSource    = NULL;
	IWICFormatConverter   *pConverter = NULL;
	IStream               *pStream    = NULL;

	do {
		hr = codecs_init();
		ASSERT_HR("codecs_init");

		if (uri) {
			TRACE("Filename %ls", uri);

			hr = pIWICFactory->CreateDecoderFromFilename(
				(LPCWSTR)uri
				, NULL
				, GENERIC_READ
				, WICDecodeMetadataCacheOnLoad
				, &pDecoder
			);
			ASSERT_HR("CreateDecoderFromFilename");
		}
		else {
			TRACE("Decoding from binary... bytes: %u", codi->len);

			// Global memory for stream will be released with the stream automaticaly
			HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, codi->len);
			if (!hMem) {
				TRACE_ERR("GlobalAlloc");
				hr = GetLastError();
				break;
			}

			hr = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
			ASSERT_HR("CreateStreamOnHGlobal");

			ULONG written;
			hr = pStream->Write(codi->data, codi->len, &written);
			ASSERT_HR("pStream->Write");

			// WIC JPEG decoder needs the stream to seek to head manually!
			// https://stackoverflow.com/a/12928336/494472
			pStream->Seek(zero, STREAM_SEEK_SET, NULL);

			hr = pIWICFactory->CreateDecoderFromStream(
				pStream
				, NULL
				, WICDecodeMetadataCacheOnLoad
				, &pDecoder
			);
			ASSERT_HR("CreateDecoderFromStream");
		}

		// Create the initial frame.
		UINT pCount = 0;
		pDecoder->GetFrameCount(&pCount);
		TRACE("Frames: %u", pCount);

		hr = pDecoder->GetFrame(frame, &pSource);
		ASSERT_HR("GetFrame");

		// Convert the image format to 32bppPBGRA
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
		ASSERT_HR("CreateFormatConverter");

		hr = pConverter->Initialize(
			pSource
			, GUID_WICPixelFormat32bppBGRA
			, WICBitmapDitherTypeNone
			, NULL
			, 0.f
			, WICBitmapPaletteTypeMedianCut
		);
		ASSERT_HR("WIC Initialize pConverter");

		pConverter->GetSize(&w, &h);
		TRACE("size: %ux%u", w, h);
		codi->w = w;
		codi->h = h;
		codi->len = w * h * 4;

		data = (unsigned char *)malloc(codi->len);
		if (data) {
			wrect = { 0,0,(INT)w,(INT)h };
			hr = pConverter->CopyPixels(&wrect, w * 4, codi->len, data);
			ASSERT_HR("CopyPixels");
		}
	} while(FALSE);

	codi->error = hr;
	if (FAILED(hr)) {
		if (data) free(data); // in case that CopyPixels fails
	}
	else {
		codi->data = data;
	}

	RELEASE(pStream);
	RELEASE(pDecoder);
    RELEASE(pSource);
	RELEASE(pConverter);

	return hr;
}

HRESULT AddBoolProperty(IPropertyBag2 *pPropertybag, LPOLESTR name, VARIANT_BOOL value) {
	PROPBAG2 option = { 0 };
	option.pstrName = name;
	VARIANT varValue;    
	VariantInit(&varValue);
	varValue.vt = VT_BOOL;
	varValue.boolVal = value;      
	return pPropertybag->Write(1, &option, &varValue);   
}

CODECS_API int EncodeImageToFile(PCWSTR *uri, REBCDI *codi)
{
	HRESULT hr = S_OK;
//	UINT  w, h;
	UINT  size;
	BYTE *data = NULL;
	WICRect wrect;

//	ULONG bytes;

	IWICBitmap         *pWICBitmap     = NULL;
	IWICBitmapLock     *pWICBitmapLock = NULL;

	IWICBitmapEncoder     *encoder     = NULL;
	IWICStream            *wicStream   = NULL;
	IWICBitmapFrameEncode *frameEncode = NULL;
	IStream               *outStream   = NULL;

	IPropertyBag2 *pPropertybag = NULL;

	GUID containerFormat;

	HGLOBAL	hMem = NULL;

	switch (codi->type) {
	case CODI_IMG_PNG:  containerFormat = GUID_ContainerFormatPng  ; break;     // Portable Network Graphics
	case CODI_IMG_JPEG: containerFormat = GUID_ContainerFormatJpeg ; break;     // Joint Photographic Experts Group
	case CODI_IMG_GIF:  containerFormat = GUID_ContainerFormatGif  ; break;     // Graphics Interchange Format
	case CODI_IMG_DDS:  containerFormat = GUID_ContainerFormatDds  ; break;     // DirectDraw Surface
	case CODI_IMG_DNG:  containerFormat = GUID_ContainerFormatAdng  ; break;    // Digital Negative
	case CODI_IMG_BMP:  containerFormat = GUID_ContainerFormatBmp  ; break;     // Device independent bitmap
	case CODI_IMG_TIFF: containerFormat = GUID_ContainerFormatTiff ; break;     // Tagged Image File Format
	case CODI_IMG_JXR:  containerFormat = GUID_ContainerFormatWmp  ; break;     // Windows Digital Photo (JpegXR)
	case CODI_IMG_HEIF: containerFormat = GUID_ContainerFormatHeif ; break;     // High Efficiency Image Format
	case CODI_IMG_WEBP: containerFormat = GUID_ContainerFormatWebp ; break;     //
	default: return WINCODEC_ERR_COMPONENTNOTFOUND;
	}

	do {
		hr = codecs_init();
		ASSERT_HR("codecs_init");

		hr = pIWICFactory->CreateBitmap(
			  codi->w
			, codi->h
			, GUID_WICPixelFormat32bppBGRA
			, WICBitmapCacheOnLoad
			, &pWICBitmap
		);
		ASSERT_HR("CreateBitmap");

		wrect = {0,0,(INT)codi->w,(INT)codi->h};

		hr = pWICBitmap->Lock(&wrect, WICBitmapLockWrite, &pWICBitmapLock);
		ASSERT_HR("WICBitmap->Lock");

		hr = pWICBitmapLock->GetDataPointer(&size, &data);
		ASSERT_HR("GetDataPointer");

		hr = memcpy_s(data, size, codi->bits, size);
		ASSERT_HR("memcpy_s to pWICBitmap");
		RELEASE(pWICBitmapLock);
		
		hr = pIWICFactory->CreateEncoder(containerFormat, nullptr, &encoder);
		ASSERT_HR("pIWICFactory->CreateEncoder");

		if (uri == NULL) {
			// output to raw binary
			hr = CreateStreamOnHGlobal(NULL, TRUE, &outStream);
			ASSERT_HR("CreateStreamOnHGlobal");
		
		} else {
			// Create a stream for the encoder
			hr = pIWICFactory->CreateStream(&wicStream);
			ASSERT_HR("pIWICFactory->CreateStream");
			// Initialize the stream using the output file path
			hr = wicStream->InitializeFromFilename((LPCWSTR)uri, GENERIC_WRITE);
			ASSERT_HR("wicStream->InitializeFromFilename");
			hr = wicStream->QueryInterface(IID_PPV_ARGS(&outStream));
			ASSERT_HR("wicStream->QueryInterface");
		}
		// Create encoder to write to image file
		encoder->Initialize(outStream, WICBitmapEncoderNoCache);

		hr = encoder->CreateNewFrame(&frameEncode, &pPropertybag);
		ASSERT_HR("encoder->CreateNewFrame");

		//TODO: handle optional per-codec properties here!

		if (codi->type == CODI_IMG_BMP) {
			// force BMP encoder to always write alpha channel so far
			hr = AddBoolProperty(pPropertybag, (LPOLESTR)L"EnableV5Header32bppBGRA", VARIANT_TRUE);
			ASSERT_HR("Set EnableV5Header32bppBGRA");
		}

		//Initialize the encoder
		hr = frameEncode->Initialize(pPropertybag);
		ASSERT_HR("frameEncode->Initialize");
		frameEncode->SetSize(codi->w, codi->h);

		// Copy updated bitmap to output
		hr = frameEncode->WriteSource(pWICBitmap, nullptr);
		ASSERT_HR("frameEncode->WriteSource");
		hr = frameEncode->Commit();
		if(FAILED(hr)) break;
		hr = encoder->Commit();
		if(FAILED(hr)) break;

		outStream->Commit(STGC_DEFAULT);
		RELEASE(encoder);

		if (uri == NULL) {
			// copy data from stream to codi
			STATSTG stat;
			hr = outStream->Stat(&stat, STATFLAG_NONAME);
			ASSERT_HR("outStream->Stat");
			if (stat.cbSize.HighPart > 0 || stat.cbSize.LowPart > UINT_MAX) {
				hr = E_OUTOFMEMORY;
				break;
			}
			TRACE("Result has %lu bytes", stat.cbSize.LowPart);

			if (codi->len < stat.cbSize.LowPart) {
				// result binary series is not large enough!

				// TODO: we could allocate a new result buffer now and let the C side to
				// know, that result binary must be extended and data copied and freed.
				// or result could keep the stream, let C know, that more extension is needed,
				// than go back here, read data and release the stream.
				hr = E_OUTOFMEMORY;
				break;
			}

			ULONG read;
			outStream->Seek(zero, STREAM_SEEK_SET, NULL);
			hr = outStream->Read(codi->data, stat.cbSize.LowPart, &read);
			ASSERT_HR("outStream->Read");
			codi->len = read;
		}
	} while(FALSE);

	RELEASE(pPropertybag);
	RELEASE(pWICBitmap);
	RELEASE(pWICBitmapLock);
	RELEASE(outStream);
	RELEASE(frameEncode);
	RELEASE(wicStream);
	RELEASE(encoder);

	codi->error = hr;

	return hr;
}
