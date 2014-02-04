// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

namespace pd {

void *&bq_spec(unsigned int id, void *&non_bq);

unsigned int bq_spec_reserve() throw();

template<typename x_t, void **non_bq>
class bq_spec_t {
	unsigned int id;

public:
	inline bq_spec_t() throw() : id(bq_spec_reserve()) { }
	inline ~bq_spec_t() throw() { }

	inline operator x_t *&() const { return (x_t *&)bq_spec(id, *non_bq); }

	inline x_t *&operator=(x_t *_x) {
		x_t *&x = operator x_t *&();
		x = _x;
		return x;
	}
};

#define bq_spec_decl(type, name) \
	namespace { __thread void *name##_non_bq = NULL; } \
	static ::pd::bq_spec_t<type, &name##_non_bq> name

} // namespace pd

#pragma GCC visibility pop
