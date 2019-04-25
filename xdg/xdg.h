#ifndef __chck_xdg__
#define __chck_xdg__

#if __GNUC__
#  if !defined(likely) && !defined(unlikely)
#     define likely(x) __builtin_expect(!!(x), 1)
#     define unlikely(x) __builtin_expect(!!(x), 0)
#  endif
#  define CHCK_FORMAT(f, x, y) __attribute__((format(f, x, y)))
#  define CHCK_MALLOC __attribute__((malloc))
#else
#  if !defined(likely) && !defined(unlikely)
#     define likely(x) !!(x)
#     define unlikely(x) !!(x)
#  endif
#  define CHCK_FORMAT(f, x, y)
#  define CHCK_MALLOC
#endif
#include <stdint.h>

struct xdg_paths {
   const char *path;
   const char *paths;
   uint32_t iter;
};

char* xdg_get_path(const char *xdg_env, const char *default_path);
const char* xdg_get_paths(const char *xdg_env, const char *default_paths, struct xdg_paths *state, uint32_t max_iter);

#endif /* __chck_xdg__ */
