#ifndef __DYNGL_H__INCLUDED__
#define __DYNGL_H__INCLUDED__

#include "GL/gl.h"
#include "GL/wgl.h"

#define DGL_ITEM(_ret, _call, _name, _args) \
	typedef _ret (_call * dyngl_f_ ## _name) _args;

#include "dyngl_list.h"

#undef DGL_ITEM

#define DGL_ITEM(_ret, _call, _name, _args) \
	dyngl_f_ ## _name _name;

typedef struct _dyngl_t
{
	HMODULE thislib;
	HMODULE lib;
	#include "dyngl_list.h"
} dyngl_t;

#undef DGL_ITEM

extern dyngl_t dyngl;

#define DGL(_func) dyngl._func

#ifdef __cplusplus
extern "C" {
#endif

BOOL dyngl_load();
void dyngl_init(HMODULE thislib);
void dyngl_destroy();

#ifdef __cplusplus
}
#endif

#endif /* __DYNGL_H__INCLUDED__ */
