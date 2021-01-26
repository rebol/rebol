#include <assert.h>
#include <windows.h>
#include <wincodecsdk.h> // for saving image

#undef IS_ERROR // not used MS version (from windows.h)

#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>


#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dcomp")
#include "sys-d2d.h"
#include "sys-codecs.h"

//typedef struct WIN_GOB WIN_GOB_t;
struct WINGOB_CTX {
	ID2D1RenderTarget     *pRenderTarget;
	ID2D1DCRenderTarget   *pDCRenderTarget;
	ID2D1SolidColorBrush  *pBrush;
	ID2D1Bitmap           *pBitmap;
	IDXGISwapChain1       *swapChain;
	ID2D1DeviceContext    *pDc          = NULL;
	IDXGISurface2         *pSurface     = NULL;
	//ID2D1Bitmap1          *pDCBitmap    = NULL;

	IWICBitmap *pWICBitmap = NULL;
	ID2D1GdiInteropRenderTarget *pInteropTarget;
};

static ID2D1Factory2      *pIFactory       = NULL;
static ID3D11Device       *pDirect3dDevice = NULL;
static IDXGIDevice        *pDxgiDevice     = NULL;
static IDXGIFactory2      *pDxFactory      = NULL;
static ID2D1Device1       *pD2Device       = NULL;
extern IWICImagingFactory *pIWICFactory;


REB_D2D_API int d2d_init ()
{
	HRESULT hr;

	TRACE("d2d init");
	CoInitialize(0);

	hr = D3D11CreateDevice(nullptr,    // Adapter
                     D3D_DRIVER_TYPE_HARDWARE,
                     nullptr,    // Module
                     D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                     nullptr, 0, // Highest available feature level
                     D3D11_SDK_VERSION,
                     &pDirect3dDevice,
                     nullptr,    // Actual feature level
                     nullptr);  // Device context
	if (FAILED(hr))
	{
		TRACE_HR("D3D11 init failed!");
		return S_FALSE;
	}
	hr = pDirect3dDevice->QueryInterface(&pDxgiDevice);
	if (FAILED(hr))
	{
		TRACE_HR("pDxgiDevice init failed!");
		return S_FALSE;
	}
	hr = CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG,
		IID_PPV_ARGS(&pDxFactory));
	if (FAILED(hr))
	{
		TRACE_HR("CreateDXGIFactory2 init failed!");
		return S_FALSE;
	}

	// Create a single-threaded Direct2D factory with debugging information
	D2D1_FACTORY_OPTIONS const options = { D2D1_DEBUG_LEVEL_INFORMATION };
	if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &pIFactory)))
	{
		TRACE("d2d init failed!");
		return S_FALSE;  // Fail CreateWindowEx.
	}

	// Create the Direct2D device that links back to the Direct3D device
	hr = pIFactory->CreateDevice(pDxgiDevice, &pD2Device);
	if (FAILED(hr))
	{
		TRACE_HR("pD2Device init failed!");
		return S_FALSE;
	}

	return S_OK;
}

REB_D2D_API void d2d_fini ()
{
	TRACE("d2d fini");
	RELEASE(pIFactory);
	RELEASE(pDxFactory);
	RELEASE(pDxgiDevice);
	RELEASE(pDirect3dDevice);
	RELEASE(pD2Device);
	CoUninitialize();
}

#ifdef unused

//TODO: find better name once settled 
REB_D2D_API void ReleaseWinGobContext(WINGOB *gob)
{
	if(!gob || !gob->ctx) return;
	WINGOB_CTX *ctx = (WINGOB_CTX *)gob->ctx;
	RELEASE(ctx->pRenderTarget);
	RELEASE(ctx->pDCRenderTarget);
	RELEASE(ctx->pBrush);
	RELEASE(ctx->pBitmap);
	RELEASE(ctx->swapChain);
	RELEASE(ctx->pDc);
	RELEASE(ctx->pSurface);
	//RELEASE(ctx->pDCBitmap);
	RELEASE(ctx->pInteropTarget);
	free(ctx);
	gob->ctx = NULL;
}


REB_D2D_API int InitLayeredWindow(WINGOB *gob)
{
	HRESULT hr = S_OK;
	WINGOB_CTX *ctx;
	IWICFormatConverter *pConverter = NULL;

	TRACE_PTR("InitLayeredWindow gob:", gob);

	if (!gob || !gob->hwnd) return S_FALSE;

	ctx = ALLOC_NEW(WINGOB_CTX);
	assert(ctx != NULL);

	SET_GOB_CTX(gob, ctx);
	//TRACE_PTR("InitLayeredWindow ctx:", gob->ctx);

	do {
		hr = codecs_init();
		ASSERT_HR("codecs_init");

		hr = pIWICFactory->CreateBitmap(
			gob->size.x,
			gob->size.y,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&ctx->pWICBitmap
		);
		ASSERT_HR("pIWICFactory->CreateBitmap")

		const D2D1_PIXEL_FORMAT format = 
		  D2D1::PixelFormat(
		  DXGI_FORMAT_B8G8R8A8_UNORM,
		  D2D1_ALPHA_MODE_PREMULTIPLIED);

		const D2D1_RENDER_TARGET_PROPERTIES properties = 
		  D2D1::RenderTargetProperties(
		  D2D1_RENDER_TARGET_TYPE_DEFAULT,
		  format,
		  96.0f, // default dpi
		  96.0f, // default dpi
		  D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

		// create the DC render target using the Direct2D factory object:
		hr = pIFactory->CreateWicBitmapRenderTarget(ctx->pWICBitmap, properties, &ctx->pRenderTarget);

		// init the ID2D1GdiInteropRenderTarget interface for GetDC and ReleaseDC
		hr = ctx->pRenderTarget->QueryInterface(&ctx->pInteropTarget);
		ASSERT_HR("QueryInterface(&ctx->pInteropTarget)")

		const wchar_t* uri = L"x:/Plan31.png";
		hr = LoadBitmapFromFile(uri , &pConverter);

		//ctx->pRenderTarget, 
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = ctx->pRenderTarget->CreateBitmapFromWicBitmap(
		    pConverter
		    , NULL
		    , &ctx->pBitmap
		);

		ASSERT_HR("image not loaded!")

		hr = UpdateLayered(gob);
	} while(FALSE);

	RELEASE(pConverter);
	
	return hr;
}

REB_D2D_API int ResizeLayered(WINGOB *gob, UINT width, UINT height)
{
	HRESULT hr = S_OK;
    WINGOB_CTX *ctx;

	TRACE("Base resize: %i %i", width, height);

	if (!gob || !gob->hwnd) return S_FALSE;
	if (!gob->ctx) InitLayeredWindow(gob);
	ctx = GET_GOB_CTX(gob); GOB_FLAGS
	return hr;
}

REB_D2D_API int UpdateLayered(WINGOB *gob)
{
	HRESULT hr;
    WINGOB_CTX *ctx;
	HDC dc;
	RECT rect = {};

	if (!gob || !gob->hwnd) return S_FALSE;
	if (!gob->ctx) InitLayeredWindow(gob);
	ctx = GET_GOB_CTX(gob);

	ctx->pRenderTarget->BeginDraw();
	ctx->pRenderTarget->Clear();

	ID2D1SolidColorBrush *brush = NULL;
	D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f, 0.55f, 0.84f, 0.1f); // rgba

	do {
		hr = ctx->pRenderTarget->CreateSolidColorBrush(brushColor, &brush);
		ASSERT_HR("CreateSolidColorBrush");

		ctx->pRenderTarget->FillRectangle(
			D2D1::RectF(20.0f,20.0f,600.0f,400.0f), brush);
		ctx->pRenderTarget->FillRectangle(
				D2D1::RectF(200.0f,200.0f,700.0f,500.0f), brush);
		if (ctx->pBitmap) {
			//SET_RECT(rect, 0,0, 100,100)
			ctx->pRenderTarget->DrawBitmap(
				ctx->pBitmap,
				D2D1::RectF(0,0,gob->size.x,0.5 * gob->size.y),
				1.0,
				D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
				//, NULL, 0.5
			);
		}

		hr = ctx->pInteropTarget->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &dc);
		ASSERT_HR("pInteropTarget->GetDC")

		// Call UpdateLayeredWindow
		BLENDFUNCTION blend = {0};
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = gob->alpha;
		blend.AlphaFormat = AC_SRC_ALPHA;
		POINT ptPos = {gob->offset.x, gob->offset.y};
		SIZE sizeWnd = {gob->size.x, gob->size.y};
		POINT ptSrc = {0, 0};
		UPDATELAYEREDWINDOWINFO info;
		info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
		info.pptSrc = &ptSrc;
		info.pptDst = &ptPos;
		info.psize = &sizeWnd;
		info.pblend = &blend;
		info.dwFlags = ULW_ALPHA;
		info.hdcSrc = dc;

		BOOL b = ::UpdateLayeredWindow(gob->hwnd, NULL, &ptPos, &sizeWnd, dc, &ptSrc, RGB(0,255,255), &blend, ULW_ALPHA);
		if(b == 0) {
			TRACE_ERR("UpdateLayeredWindow ERROR:");
		}
		//UpdateLayeredWindowIndirect(gob->hwnd, &info);
		ctx->pInteropTarget->ReleaseDC({});

		hr = ctx->pRenderTarget->EndDraw();
		ASSERT_HR("ctx->pRenderTarget->EndDraw:");
	} while(FALSE);

	RELEASE(brush);

//	if (ctx->pWICBitmap) {
//		UINT w, h;
//		ctx->pWICBitmap->GetSize(&w, &h);
//		TRACE("WICBitmap size: %ux%u", w, h);
//		SaveBitmap(ctx->pWICBitmap, L"x:/test-out.png");
//	}
	return hr;
}

#endif