#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <any>

#include <windows.h>
#include <winerror.h>
#include <errhandlingapi.h>

#define IWDEM_CheckOrReturn(expr, retval, ...) \
	do {                                       \
		if (!(expr)) {                         \
			do {                               \
				__VA_ARGS__;                   \
			} while (0);                       \
			return (retval);                   \
		}                                      \
	} while (0);
