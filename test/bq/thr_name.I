#include <pd/base/string.H>

#include <sys/prctl.h>

namespace pd {

inline string_t thr_name() {
	char buf[17];
	prctl(PR_GET_NAME, buf);
	buf[16] = '\0';

	return string_t::ctor_t(16)(str_t(buf, strlen(buf)));
}

} // namespace pd
