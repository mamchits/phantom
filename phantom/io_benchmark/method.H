// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared.H>

#include <phantom/io_benchmark/times.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_benchmark {

class method_t : public shared_t {
public:
	virtual bool test(times_t &times) const = 0;

protected:
	inline method_t(string_t const &name) throw() : shared_t(name) { }
	inline ~method_t() throw() { }
};

}} // namespace phantom::io_benchmark

#pragma GCC visibility pop
