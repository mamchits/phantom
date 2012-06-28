/*
  fixing wrong libc's pthread_self() attribute((const))
 */

#ifndef __bq_pthread_self_workaround_disabled

#ifndef __bq_pthread_self_workaround
#define __bq_pthread_self_workaround
#endif

#define pthread_self pthread_self_wrong_prototype
#include_next <pthread.h>
#undef pthread_self

__BEGIN_DECLS

pthread_t pthread_self(void) __THROW;

__END_DECLS

#else

#include_next <pthread.h>

#endif
