// This file is part of the phantom::io_client module.
// Copyright (C) 2010-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2010-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <phantom/shared.H>

#include <pd/base/fd.H>
#include <pd/base/string.H>

#include <pd/bq/bq_conn.H>

#pragma GCC visibility push(default)

namespace phantom { namespace io_client {

class proto_t : public shared_t {
public:
	class instance_t {
	protected:
		unsigned int drank;
	public:
		virtual ~instance_t() throw();
		virtual void init() = 0;
		virtual void proc(bq_conn_t &conn) = 0;
		virtual void stat_print() = 0;

	protected:
		inline instance_t(unsigned int _rank) : drank(_rank) { }
	};

	virtual instance_t *create_instance(unsigned int _rank) = 0;

protected:
	inline proto_t(string_t const &name) throw() : shared_t(name) { }
	inline ~proto_t() throw() { }
};

}} // namespace phantom::io_client

#pragma GCC visibility pop
