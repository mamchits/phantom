// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/pd.H>

#include <pd/base/fd.H>
#include <pd/base/string.H>

#include <pd/bq/bq_conn.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

class proto_instance_t {
public:
	virtual ~proto_instance_t() throw();
	virtual void proc(bq_conn_t &conn) = 0;

protected:
	inline proto_instance_t() { }
};

class proto_t {
public:
	virtual void init() = 0;
	virtual void run() = 0;
	virtual proto_instance_t *create_instance(string_t const &name) = 0;
	virtual void stat(out_t &out, bool clear) = 0;

protected:
	inline proto_t() throw() { }
	inline ~proto_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace phantom::io_client

#pragma GCC visibility pop
