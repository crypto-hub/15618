#ifndef __UTILS_H__
#define __UTILS_H__


//#define DEBUG

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)

#else
#define dbg_printf(...)
#endif



#endif