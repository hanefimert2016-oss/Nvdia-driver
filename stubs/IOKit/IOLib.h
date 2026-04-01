/*
 * stubs/IOKit/IOLib.h
 * Minimal IOKit stubs for syntax-checking on Linux CI.
 * Not used on macOS – the real SDK headers take precedence.
 */
#ifndef _IOKIT_IOLIB_H_STUB
#define _IOKIT_IOLIB_H_STUB

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOLog(fmt, ...) printf(fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _IOKIT_IOLIB_H_STUB */
