#include "xdg.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

/** clang feature detection. */
#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#if __GNUC__ >= 5 || __has_builtin(__builtin_add_overflow)
/** assume clang and gcc (>=5) to only have builtins for now. */
#  define add_of(a, b, r) __builtin_add_overflow(a, b, r)
#  define sub_of(a, b, r) __builtin_sub_overflow(a, b, r)
#  define mul_of(a, b, r) __builtin_mul_overflow(a, b, r)
#  define of_attr
#else
/** else use these generics, note behaviour of these is not strictly defined. */
#  define add_of(a, b, r) ((*(r) = ((a) + (b))) < (a))
#  define sub_of(a, b, r) ((*(r) = ((a) - (b))) > (a))
#  define mul_of(a, b, r) (((*(r) = ((a) * (b))) || *(r) == 0) && ((a) != 0 && (b) > *(r) / (a)))
#  if __clang__
#     define of_attr __attribute__((optnone)) // Do not optimize above checks, in most systems this will work, but not defined.
#     warning "Using non compiler builtins for overflow checks, this will be undefined for signed integers"
#  elif __GNUC__
#     define of_attr __attribute__((optimize("wrapv"))) // in older GCC we can make this behavior defined
#  else
#     warning "Using non compiler builtins for overflow checks, this will be undefined for signed integers"
#  endif
#endif

/** declare overflow functions */

// T = type name, n = function suffix, s = is type signed?
#define decl_generics_for_type(T, n, s) \
   of_attr static inline bool chck_add_of##n(T a, T b, T *r) { assert((!s || b >= 0) && r); return add_of(a, b, r); } \
   of_attr static inline bool chck_sub_of##n(T a, T b, T *r) { assert((!s || b >= 0) && r); return sub_of(a, b, r); } \
   of_attr static inline bool chck_mul_of##n(T a, T b, T *r) { assert(r); return mul_of(a, b, r); }

// UT = unsigned type, un = unsigned function suffix
// T = signed type, n = signed function suffix
#define decl_generics(UT, un, T, n) \
   decl_generics_for_type(UT, un, false) \
   decl_generics_for_type(T, n, true)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"

decl_generics_for_type(size_t, sz, false)
decl_generics(uint64_t, u64, int64_t, 64)
decl_generics(uint32_t, u32, int32_t, 32)
decl_generics(uint16_t, u16, int16_t, 16)
decl_generics(uint8_t, u8, int8_t, 8)

#pragma GCC diagnostic pop

#undef decl_generics
#undef decl_generics_for_type
#undef add_of
#undef sub_of
#undef mul_of
#undef of_attr

CHCK_MALLOC static inline void*
chck_malloc_add_of(size_t size, size_t add)
{
   size_t r;
   if (unlikely(chck_add_ofsz(size, add, &r)) || !r)
      return NULL;

   return malloc(r);
}

CHCK_MALLOC static inline void*
chck_malloc_sub_of(size_t size, size_t sub)
{
   size_t r;
   if (unlikely(chck_sub_ofsz(size, sub, &r)) || !r)
      return NULL;

   return malloc(r);
}

CHCK_MALLOC static inline void*
chck_malloc_mul_of(size_t size, size_t mul)
{
   size_t r;
   if (unlikely(chck_mul_ofsz(size, mul, &r)) || !r)
      return NULL;

   return malloc(r);
}

CHCK_MALLOC static inline void*
chck_calloc_of(size_t nmemb, size_t size)
{
   size_t r;
   if (unlikely(chck_mul_ofsz(nmemb, size, &r)) || !r)
      return NULL;

   return calloc(nmemb, size);
}

CHCK_MALLOC static inline void*
chck_calloc_add_of(size_t size, size_t add)
{
   size_t r;
   if (unlikely(chck_add_ofsz(size, add, &r)) || !r)
      return NULL;

   return calloc(1, r);
}

CHCK_MALLOC static inline void*
chck_calloc_sub_of(size_t size, size_t sub)
{
   size_t r;
   if (unlikely(chck_sub_ofsz(size, sub, &r)) || !r)
      return NULL;

   return calloc(1, r);
}

static inline void*
chck_realloc_add_of(void *ptr, size_t size, size_t add)
{
   size_t r;
   if (unlikely(chck_add_ofsz(size, add, &r)) || !r)
      return NULL;

   return realloc(ptr, r);
}

static inline void*
chck_realloc_sub_of(void *ptr, size_t size, size_t sub)
{
   size_t r;
   if (unlikely(chck_sub_ofsz(size, sub, &r)) || !r)
      return NULL;

   return realloc(ptr, r);
}

static inline void*
chck_realloc_mul_of(void *ptr, size_t size, size_t mul)
{
   size_t r;
   if (unlikely(chck_mul_ofsz(size, mul, &r)) || !r)
      return NULL;

   return realloc(ptr, r);
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pwd.h>

static inline char*
ccopy(const char *str)
{
   assert(str);
   const size_t size = strlen(str);
   char *cpy = chck_calloc_add_of(size, 1);
   return (cpy ? memcpy(cpy, str, size) : NULL);
}

static inline char*
strip_slash(char *str)
{
   // if you get invalid read of size 4 here with valgrind
   // it means that you compiled with O2 or higher optimizations on gcc
   // gcc has builtin optimized strlen that reads bytes in aligned chunks

   size_t size;
   if (!str || !(size = strlen(str)))
      return NULL;

   for (char *s = str + size - 1; s >= str && *s == '/'; --s)
      *s = 0;

   return str;
}

static inline char*
get_home(void)
{
   const char *env;
   if ((env = getenv("HOME")) && env[0] == '/')
      return ccopy(env);

   struct passwd *pwd;
   if (!(pwd = getpwuid(getuid())) || !pwd->pw_dir || pwd->pw_dir[0] != '/')
      return NULL;

   return ccopy(pwd->pw_dir);
}

char*
xdg_get_path(const char *xdg_env, const char *default_path)
{
   assert(xdg_env && default_path && default_path[0] != '/');

   if (!xdg_env || !default_path || default_path[0] == '/')
      return NULL;

   const char *xdg_dir;
   if ((xdg_dir = getenv(xdg_env)) && xdg_dir[0] == '/')
      return strip_slash(ccopy(xdg_dir));

   char *home;
   if (!(home = get_home()))
      return NULL; /** fatal! */

   const size_t len = snprintf(NULL, 0, "%s/%s", home, default_path) + 1;

   char *path;
   if (!(path = calloc(1, len))) {
      free(home);
      return NULL; /** fatal! */
   }

   snprintf(path, len, "%s/%s", home, default_path);
   free(home);
   return path;
}

const char*
xdg_get_paths(const char *xdg_env, const char *default_paths, struct xdg_paths *state, uint32_t max_iter)
{
   assert(xdg_env && default_paths && state);

   if ((state->path && !*state->path) || state->iter == max_iter) {
      free((char*)state->paths);
      return NULL;
   }

   ++state->iter;

   if (!state->paths) {
      if (!xdg_env || !default_paths)
         return NULL;

      const char *paths;
      if (!(paths = getenv(xdg_env)) || !paths[0])
         paths = default_paths;

      state->path = state->paths = ccopy(paths);
   }

   if (!state->path || !state->paths)
      return NULL;

   char *path;
   do {
      size_t f;
      path = (char*)state->path;
      if ((f = strcspn(state->path, ":")) > 0) {
         state->path += f + (path[f] ? 1 : 0);
         path[f] = 0;
      }

      if (!*path) {
         free((char*)state->paths);
         return NULL;
      }
   } while (path[0] != '/');

   return strip_slash(path);
}
