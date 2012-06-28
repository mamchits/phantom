// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>

#include <stdarg.h>

#pragma GCC visibility push(default)

namespace pd { namespace log {

enum level_t { debug = 0, info, warning, error, trace = 0x80 };

inline level_t operator|(level_t l1, level_t l2) throw() {
	return (level_t)(((unsigned int)l1) | ((unsigned int)l2));
}

inline level_t operator&(level_t l1, level_t l2) throw() {
	return (level_t)(((unsigned int)l1) & ((unsigned int)l2));
}

inline level_t operator~(level_t l) throw() {
	return (level_t)(~(unsigned int)l);
}

class backend_t {
protected:
	inline backend_t() throw() { }
	inline ~backend_t() throw() { }

public:
	virtual void commit(iovec const *iov, size_t count) const throw() = 0;

	inline void commit(char const *str, size_t len) const throw() {
		iovec iov = (iovec) { (void *)str, len };
		commit(&iov, 1);
	}

	virtual level_t level() const throw() = 0;

private: // don't use
	backend_t(backend_t const &);
	backend_t &operator=(backend_t const &);
};

class aux_t {
public:
	virtual void print(out_t &out) const = 0;
	inline aux_t() throw() { }
	virtual ~aux_t() throw();
};

class handler_t {
	handler_t const *prev;

protected:
	string_t const label;
	backend_t const &backend;

public:
	inline level_t level() const throw() { return backend.level(); }

	virtual void vput(
		level_t level, aux_t const *aux, char const *format, va_list vargs
	) const throw() = 0;

	static void init(
		void (*_set)(handler_t const *), handler_t const *(*_get)()
	) throw();

	static handler_t const *get() throw();
	static void set(handler_t const *) throw();

	static backend_t const &get_backend() throw() {
		return get()->backend;
	}

	static string_t const &get_label() throw() {
		return get()->label;
	}

	inline handler_t(string_t const &_label, backend_t const *_backend) throw() :
		prev(get()), label(_label),
		backend(_backend ? *_backend : prev->backend) { set(this); }

	inline void __format(4) put(
		level_t level, aux_t const *aux, char const *format, ...
	) const throw() {
		if(backend.level() > level)
			return;

		va_list args;
		va_start(args, format);
		vput(level, aux, format, args);
		va_end(args);
	}

	inline ~handler_t() throw() { set(prev); }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

#define log_put(lev, aux, format...) \
	do { \
		log::handler_t const *handler = log::handler_t::get(); \
		if(handler && handler->level() <= lev) \
			handler->put(lev, aux, format); \
	} while(0)

#define log_debug(format...) log_put(log::debug, NULL, format)
#define log_info(format...) log_put(log::info, NULL, format)
#define log_warning(format...) log_put(log::warning, NULL, format)
#define log_error(format...) log_put(log::error, NULL, format)

class handler_default_t : public handler_t {
	virtual void vput(
		level_t level, aux_t const *aux, char const *format, va_list vargs
	) const throw();

public:
	inline handler_default_t(
		string_t const &_label, backend_t const *_backend = NULL
	) throw() : handler_t(_label, _backend) { }

	inline ~handler_default_t() throw() { }

private: // don't use
	void *operator new(size_t);
	void operator delete(void *);
};

}} // namespace pd::log

#pragma GCC visibility pop
