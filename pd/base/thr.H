// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/stat.H>

#pragma GCC visibility push(default)

namespace pd { namespace thr {

enum state_t { idle, run, locked, mmap };

struct tstate_t : stat::tstate_t<state_t, 4> {
	inline tstate_t() :
		stat::tstate_t<state_t, 4> (
			STRING("idle"), STRING("run"), STRING("locked"), STRING("mmap")
		) { }
};

extern __thread tstate_t *tstate;

extern __thread pid_t id;

}} // namespace pd::thr

#pragma GCC visibility pop
