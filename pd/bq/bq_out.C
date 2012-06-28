// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_out.H"
#include "bq_util.H"

#include <pd/base/exception.H>

#include <unistd.h>

namespace pd {

void bq_out_t::flush() {
	iovec outvec[2]; int outcount = 0;

	if(rpos < size) {
		outvec[outcount].iov_base = data + rpos;
		outvec[outcount].iov_len = ((wpos > rpos) ? wpos : size) - rpos;
		++outcount;
	}

	if(wpos > 0 && wpos <= rpos) {
		outvec[outcount].iov_base = data;
		outvec[outcount].iov_len = wpos;
		++outcount;
	}

	if(outcount) {
		size_t res = conn.writev(outvec, outcount, &timeout_cur);

		rpos += res;
		if(rpos > size) rpos -= size;
	}

	if(rpos == wpos) {
		wpos = 0;
		rpos = size;
	}
}

out_t &bq_out_t::ctl(int i) {
	conn.ctl(i);
	return *this;
}

out_t &bq_out_t::sendfile(int from_fd, off_t &_offset, size_t &_len) {
	flush_all();
	conn.sendfile(from_fd, _offset, _len, &timeout_cur);
	return *this;
}

bq_out_t::~bq_out_t() throw() {
	safe_run(*this, &bq_out_t::flush_all);
}

} // namespace pd
