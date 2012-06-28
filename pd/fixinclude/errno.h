/*
	fixing wrong libc's __errno_location() attribute((const))

	See bug_glibc_errno.C
                     -Eugene
 */

#ifndef __bq_errno_workaround_disabled

#ifndef __bq_errno_workaround
#define __bq_errno_workaround
#endif

#define __errno_location __errno_location_wrong_prototype
#include_next <errno.h>
#undef __errno_location

__BEGIN_DECLS

int *__errno_location(void) __THROW;

__END_DECLS

#else

#include_next <errno.h>

#endif
