
#include "utils.hpp"

#include <string>
#include <stdarg.h>

std::string stringf(const std::string fmt, ...) {
	// use a rubric appropriate for your code
	int n, size;
	std::string str;
	va_list ap;

	va_start(ap, fmt);
	n = vsnprintf((char *)'\0', 0, fmt.c_str(), ap);
	va_end(ap);

	size = n + 1;
	// maximum 2 passes on a POSIX system...
	while (1) {
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char *) str.data(), size, fmt.c_str(), ap);
		va_end(ap);
		// everything worked
		if (n > -1 && n < size) {
			str.resize(n);
			return str;
		}
		// needed size returned
		if (n > -1) {
			// for null char
			size = n + 1;
		} else {
			// guess at a larger size (o/s specific)
			size *= 2;
		}
	}
	return str;
}

