#define REB_D2D_API // nothing so far

#include "reb-host.h"
#include "host-lib.h"
#include "reb-types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "sys-utils.h"
//#include "types.h"



typedef struct win_gob WINGOB;

struct win_gob {
	REBCNT flags;		// option flags
	REBCNT id;
	REBXYI size;
	REBXYI offset;
	HWND   hwnd;
	REBYTE alpha;
	REBYTE ctype;		// content data type
	REBUPT *ctx;
	WINGOB *prev;
	WINGOB *next;
	WINGOB *parent;
	WINGOB *child;
};

REB_D2D_API int  d2d_init();
REB_D2D_API void d2d_fini();
REB_D2D_API int  InitLayeredWindow(WINGOB *gob);
REB_D2D_API void ReleaseWinGobContext(WINGOB *gob);
REB_D2D_API int  ResizeLayered(WINGOB *gob, UINT width, UINT height);
REB_D2D_API int  UpdateLayered(WINGOB *gob);

#ifdef __cplusplus
} // extern "C" {
#endif