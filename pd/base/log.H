// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
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

	backend_t(backend_t const &) = delete;
	backend_t &operator=(backend_t const &) = delete;

public:
	virtual void commit(iovec const *iov, size_t count) const throw() = 0;

	inline void commit(char const *str, size_t len) const throw() {
		iovec iov = (iovec) { (void *)str, len };
		commit(&iov, 1);
	}

	virtual level_t level() const throw() = 0;
};

class aux_t {
public:
	virtual void print(out_t &out) const = 0;
	inline aux_t() throw() { }
	virtual ~aux_t() throw();
};

class handler_base_t {
	handler_base_t *prev;
	bool root;

protected:
	string_t const _label;
	backend_t const &_backend;

public:
	inline level_t level() const throw() { return _backend.level(); }

	size_t print_label(out_t *out, bool slash = false) const;

	virtual void vput(
		level_t level, aux_t const *aux, char const *format, va_list vargs
	) const throw() = 0;

	typedef handler_base_t *&(*current_funct_t)();

	static current_funct_t setup(current_funct_t _current_funct) throw();
	static handler_base_t *&current() throw();

	static inline backend_t const &backend() throw() { return current()->_backend; }
	static inline string_t const &label() throw() { return current()->_label; }

	inline handler_base_t(
		string_t const &__label, backend_t const *__backend, bool _root
	) throw() :
		prev(({
			handler_base_t *&cur = current();
			handler_base_t *res = cur;
			cur = this;
			res;
		})), root(_root), _label(__label),
		_backend(__backend ? *__backend : prev->_backend) { }

	inline void __format(4) put(
		level_t level, aux_t const *aux, char const *format, ...
	) throw() {
		if(_backend.level() > level)
			return;

		va_list args;
		va_start(args, format);
		vput(level, aux, format, args);
		va_end(args);
	}

	inline ~handler_base_t() throw() { current() = prev; }

	void *operator new(size_t) = delete;
	void operator delete(void *) = delete;
};

#define log_put(lev, aux, format...) \
	do { \
		::pd::log::handler_base_t *handler = ::pd::log::handler_t::current(); \
		if(handler && handler->level() <= lev) \
			handler->put(lev, aux, format); \
	} while(0)

#define log_debug(format...) log_put(::pd::log::debug, NULL, format)
#define log_info(format...) log_put(::pd::log::info, NULL, format)
#define log_warning(format...) log_put(::pd::log::warning, NULL, format)
#define log_error(format...) log_put(::pd::log::error, NULL, format)

inline char const *number_fmt(size_t cnt) {
	switch(cnt) {
		case 0 ... 10: return "01";
		case 11 ... 100: return "02";
		case 101 ... 1000: return "03";
		case 1001 ... 10000: return "04";
		case 10001 ... 100000: return "05";
	}
	return "06";
}

class handler_t : public handler_base_t {
	virtual void vput(
		level_t level, aux_t const *aux, char const *format, va_list vargs
	) const throw();

public:
	inline handler_t(
		string_t const &_label, backend_t const *_backend = NULL, bool root = false
	) throw() : handler_base_t(_label, _backend, root) { }

	inline ~handler_t() throw() { }
};

}} // namespace pd::log

#pragma GCC visibility pop
