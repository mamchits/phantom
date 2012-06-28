// This file is part of the pd::http library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/http/http.H>

#include <pd/base/netaddr.H>

#pragma GCC visibility push(default)

namespace pd { namespace http {

class remote_request_t {
public:
	in_segment_t line;
	method_t method;

	in_segment_t uri_path;
	in_segment_t uri_args;

	version_t version;

	mime_header_t header;
	entity_t entity;
	in_segment_t all;

	netaddr_t const &local_addr;
	netaddr_t const &remote_addr;

	string_t host;
	string_t full_path; // decoded uri_path
	string_t path; // decoded uri_path w/o matched location

	timeval_t time;
	string_t time_string;

	bool keepalive;

	inline remote_request_t(
		netaddr_t const &_local_addr, netaddr_t const &_remote_addr
	) throw() :
		line(), method(method_undefined), uri_path(), uri_args(),
		version(version_undefined), header(), entity(),
		local_addr(_local_addr), remote_addr(_remote_addr),
		time(timeval_never), time_string(), keepalive(false) { }

	inline ~remote_request_t() throw() { }

	void parse(in_t::ptr_t &ptr, limits_t const &limits);

	inline void settime() {
		time_string = time_current_string(time);
	}

	inline void print_header(out_t &out) {
		if(time.is_real())
			settime();

		out(CSTR("Date: "))(time_string).crlf();
	}

	void prepare(string_t const &default_host);

private: // don't use
	remote_request_t(remote_request_t const &);
	remote_request_t &operator=(remote_request_t const &);
};

class server_t {
public:
	virtual void print_header(out_t &out) const = 0;
	virtual bool filter_header(in_segment_t const &key) const = 0;

protected:
	inline server_t() throw() { }
	inline ~server_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

class local_reply_t {
public:
	remote_request_t &request;
	interval_t reply_time;

	class content_t {
		virtual code_t code() const throw() = 0;
		virtual ssize_t size() const = 0;
		virtual void print_header(out_t &, server_t const &) const = 0;
		virtual bool print(out_t &) const = 0;
		virtual void postout() const;
		virtual string_t ext_info(string_t const &) const;

	protected:
		inline content_t() throw() { }
		virtual ~content_t() throw();

		friend class local_reply_t;

	private: // don't use
		content_t(content_t &);
		content_t &operator=(content_t &);
	};

	class error_content_t : public content_t {
		code_t error_code;
		string_t descr;

		virtual code_t code() const throw();
		virtual void print_header(out_t &out, server_t const &) const;
		virtual ssize_t size() const throw();
		virtual bool print(out_t &out) const;

	public:
		inline error_content_t(code_t _error_code) throw() :
			error_code(_error_code), descr(code_descr(_error_code)) { }

		virtual ~error_content_t() throw();
	};

private:
	content_t *content;

public:
	inline void set(content_t *_content) throw() {
		if(content) delete content;
		content = _content;
	}

	inline void postout() const {
		if(content) content->postout();
	}

	inline string_t ext_info(string_t const &key) const {
		return content ? content->ext_info(key) : string_t::empty;
	}

	inline code_t code() const throw() {
		return content ? content->code() : code_undefined;
	}

	inline ssize_t size() const throw() {
		return content ? content->size() : -1;
	}

	inline local_reply_t(remote_request_t &_request) throw() :
		request(_request), reply_time(interval_zero), content(NULL) { }

	void print(out_t &out, server_t const &server);

	inline ~local_reply_t() throw() { if(content) delete content; }

private: // don't use
	local_reply_t(local_reply_t const &);
	local_reply_t &operator=(local_reply_t const &);
};

string_t path_decode(in_segment_t const &uri_path);

}} // namespace pd::http

#pragma GCC visibility pop
