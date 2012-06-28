// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <stdint.h>

#pragma GCC visibility push(default)

namespace pd {

class sizeval_t {
	uint64_t val;
public:
	inline sizeval_t() throw() : val(0) { }
	inline sizeval_t(uint64_t _val) throw() : val(_val) { }
	inline sizeval_t(sizeval_t const &s) throw() : val(s.val) { }

	inline sizeval_t &operator=(sizeval_t const &s) throw() {
		val = s.val;
		return *this;
	}

	inline sizeval_t &operator=(uint64_t _val) {
		val = _val;
		return *this;
	}

	inline ~sizeval_t() throw() { }

	inline operator uint64_t &() throw() { return val; }
	inline operator uint64_t const &() const throw() { return val; }
};

#define sizeval_unit_val ((uint64_t)1)
#define sizeval_kilo_val (sizeval_unit_val * 1024)
#define sizeval_mega_val (sizeval_kilo_val * 1024)
#define sizeval_giga_val (sizeval_mega_val * 1024)
#define sizeval_tera_val (sizeval_giga_val * 1024)
#define sizeval_peta_val (sizeval_tera_val * 1024)
#define sizeval_exa_val (sizeval_peta_val * 1024)
#define sizeval_unlimited_val (~0ULL)

#define sizeval_kilo sizeval_t(sizeval_kilo_val)
#define sizeval_mega sizeval_t(sizeval_mega_val)
#define sizeval_giga sizeval_t(sizeval_giga_val)
#define sizeval_tera sizeval_t(sizeval_tera_val)
#define sizeval_peta sizeval_t(sizeval_peta_val)
#define sizeval_exa sizeval_t(sizeval_exa_val)
#define sizeval_unlimited sizeval_t(sizeval_unlimited_val)

} // namespace pd

#pragma GCC visibility pop
