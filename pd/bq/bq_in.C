// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_in.H"
#include "bq_util.H"

#include <pd/base/exception.H>

namespace pd {

size_t bq_in_t::readv(iovec *iov, size_t cnt) {
	return conn.readv(iov, cnt, &timeout_cur);
}

bq_in_t::~bq_in_t() throw() { }

} // namespace pd
