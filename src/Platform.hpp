#pragma once

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
