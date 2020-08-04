#pragma once

#include <stdint.h>

#ifdef _WIN32
#	define AC_API __declspec(dllexport)
#	define AC_INTERNAL
#else
#	ifdef __GNUC__
#		define AC_API __attribute__((visibility("default")))
#		define AC_INTERNAL __attribute__((visibility("internal")))
#	else
#		define AC_API
#		define AC_INTERNAL
#	endif
#endif

#if defined(__M_IX86)||defined(__i386__)||defined(__amd64__)||defined(_M_AMD64)||defined(__arm__)
//Inversed negative integers
#define floorASR(value, shmat, dest) (dest=value >> shmat)
#define floorASRQ(value, shmat, dest) (dest=value >> shmat)
#else
#define floorASR(value, shmat, dest) (dest=std::floor(value / (float)(2 << shmat)))
#define floorASRQ(value, shmat, dest) (dest=std::floor(value / (float)(2 << shmat)))
#endif
