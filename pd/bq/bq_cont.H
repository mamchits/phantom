// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/defs.H>

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

struct __hidden bq_cont_t;

extern __thread bq_cont_t *bq_cont_current;

#define bq_cont_states (none)

void bq_cont_activate(bq_cont_t *cont) throw();
void bq_cont_deactivate(char const *where) throw();

void const *bq_cont_id(bq_cont_t const *cont) throw();
char const *bq_cont_where(bq_cont_t const *cont) throw();
size_t bq_cont_stack_size(bq_cont_t const *cont) throw();

struct bq_stack_pool_info_t {
	size_t wsize, size;
};

bq_stack_pool_info_t bq_stack_pool_get_info() throw();

size_t bq_cont_count() throw();

} // namespace pd

#pragma GCC visibility pop
