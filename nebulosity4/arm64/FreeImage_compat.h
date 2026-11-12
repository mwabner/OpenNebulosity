#ifndef NEBULOSITY_ARM64_FREEIMAGE_COMPAT_H
#define NEBULOSITY_ARM64_FREEIMAGE_COMPAT_H

#include "FreeImage.h"

#if defined(__APPLE__) && defined(__aarch64__)
#ifdef _WINDOWS_
#undef _WINDOWS_
#endif
#endif

#endif
