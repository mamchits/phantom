// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/bq/bq.H>

#include <pd/base/defs.H>

#include <stddef.h>

#pragma GCC visibility push(default)

namespace pd {

struct __hidden bq_cont_t;

extern __thread bq_cont_t *bq_cont_current;

enum bq_cont_state_t {
	running = 0,
	wait_another = 1,
	wait_event = 2,
	wait_ready = 3,
	wait_ext = 4,
	none = 5
};

#define bq_cont_states (none)

void bq_cont_set_msg(bq_cont_t *cont, bq_err_t msg) throw();
void bq_cont_activate(bq_cont_t *cont) throw();
bq_err_t bq_cont_deactivate(char const *where, bq_cont_state_t state) throw();
void bq_cont_stat_update(bq_cont_t *cont, bq_cont_state_t new_state) throw();

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
