// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "config_helper.H"

namespace pd {

template<typename>
struct int_type_info_t { };

template<>
struct int_type_info_t<char> {
	typedef signed char signed_t;
	typedef unsigned char unsigned_t;
};

#define __intdef(s, type) \
template<> \
struct int_type_info_t<s type> { \
	typedef signed type signed_t; \
	typedef unsigned type unsigned_t; \
};

#define __dintdef(type) \
	__intdef(signed, type) \
	__intdef(unsigned, type)

__dintdef(char)
__dintdef(short int)
__dintdef(int)
__dintdef(long int)
__dintdef(long long int)

#ifdef _LP64
__dintdef(int  __attribute__((mode(TI))))
#endif

#undef __dintdef
#undef __intdef

template<typename num_t>
static inline bool digit(char c, num_t base, num_t &res) {
	num_t _res = 0;

	switch(c) {
		case '0'...'9': _res = c - '0'; break;
		case 'a'...'f': _res = c - 'a' + 10; break;
		case 'A'...'F': _res = c - 'A' + 10; break;
		default: return false;
	}

	if(_res < base) {
		res = _res;
		return true;
	}

	return false;
}

template<typename num_t>
static inline bool parse_unsigned(
	in_t::ptr_t &ptr, num_t &num, char const *fmt, in_t::error_handler_t handler
) {
	num_t res;
	in_t::ptr_t p = ptr;
	num_t base = 10;

	if(fmt) {
		char c;
		while((c = *(fmt++))) {
			if(c == 'x' || c == 'X')
				base = 16;
		}
	}

	if(!p || !digit(*p, base, res)) {
		if(handler) (*handler)(ptr, "digit expected");
		return false;
	}

	while(true) {
		num_t tmp;
		if(!++p || !digit(*p, base, tmp)) break;

		tmp += (res * base);

		if((tmp / base) != res) {
			if(handler) (*handler)(ptr, "integer overflow");
			return false;
		}

		res = tmp;
	}

	ptr = p;
	num = res;
	return true;
}

template<typename num_t>
static inline bool parse_signed(
	in_t::ptr_t &ptr, num_t &num, char const *fmt, in_t::error_handler_t handler
) {
	in_t::ptr_t p = ptr;

	if(!p) {
		if(handler) (*handler)(ptr, "sign or digit expected");
		return false;
	}

	bool sign = false;
	switch(*p) {
		case '-': sign = true;
		case '+': ++p;
	};

	typename int_type_info_t<num_t>::unsigned_t ures;
	if(!parse_unsigned(p, ures, fmt, handler))
		return false;

	num_t res = sign ? -ures : ures;

	if((sign && res > 0) || (!sign && res < 0)) {
		if(handler) (*handler)(ptr, "integer overflow");
		return false;
	}

	ptr = p;
	num = res;
	return true;
}

template<typename num_t>
static inline void print_signed(out_t &out, num_t num, char const *fmt = NULL) {
	bool sign = false;
	char pad = ' ';
	unsigned short int width = 0;
	unsigned base = 10;
	char const *digits = "0123456789";

	if(fmt) {
		if(*fmt == '+') { sign = true; ++fmt; }
		if(*fmt == '0') { pad = '0'; ++fmt; }
		while(*fmt >= '0' && *fmt <= '9') (width *= 10) += (*(fmt++) - '0');
		switch(*fmt) {
			case 'x': { base = 16; digits="0123456789abcdef"; } break;
			case 'X': { base = 16; digits="0123456789ABCDEF"; } break;
			case 'o':
			case 'O': { base = 8; digits="01234567"; } break;
		}
	}

	unsigned int buflen = max(width, (unsigned short int)(3 * sizeof(num_t) + 1));

	char buf[buflen]; char *ptr = buf + sizeof(buf); char const *bound = ptr;

	bool nsign = false;

	typename int_type_info_t<num_t>::unsigned_t pnum;

	if(num >= 0) {
		pnum = num;
	}
	else {
		pnum = -num; nsign = true; sign = true;
	}

	do {
		*(--ptr) = digits[pnum % base]; pnum /= base;
	} while(pnum);

	if(pad != '0') {
		if(sign) *(--ptr) = nsign ? '-' : '+';
		while(bound - ptr < width) *(--ptr) = pad;
	}
	else {
		if(sign && width) --width;
		while(bound - ptr < width) *(--ptr) = pad;
		if(sign) *(--ptr) = nsign ? '-' : '+';
	}

	out(str_t(ptr, bound - ptr));
}

template<typename num_t>
static inline void print_unsigned(out_t &out, num_t num, char const *fmt = NULL) {
	char pad = ' ';
	unsigned short int width = 0;
	unsigned base = 10;
	char const *digits = "0123456789";

	if(fmt) {
		if(*fmt == '0') { pad = '0'; ++fmt; }
		while(*fmt >= '0' && *fmt <= '9') (width *= 10) += (*(fmt++) - '0');
		switch(*fmt) {
			case 'x': { base = 16; digits="0123456789abcdef"; } break;
			case 'X': { base = 16; digits="0123456789ABCDEF"; } break;
			case 'o':
			case 'O': { base = 8; digits="01234567"; } break;
		}
	}

	unsigned int buflen = max(width, (unsigned short int)(3 * sizeof(num_t)));

	char buf[buflen]; char *ptr = buf + sizeof(buf); char const *bound = ptr;

	typename int_type_info_t<num_t>::unsigned_t pnum = num;

	do {
		*(--ptr) = digits[pnum % base]; pnum /= base;
	} while(pnum);

	while(bound - ptr < width) *(--ptr) = pad;

	out(str_t(ptr, bound - ptr));
}

template<>
bool in_t::helper_t<char>::parse(
	ptr_t &ptr, char &val, char const *fmt, error_handler_t handler
) {
	return ((char)-1 < (char)1)
		? parse_signed(ptr, val, fmt, handler)
		: parse_unsigned(ptr, val, fmt, handler)
	;
}

template<>
void out_t::helper_t<char>::print(out_t &out, char const &x, char const *fmt) {
	if((char)-1 < (char)1)
		print_signed(out, x, fmt);
	else
		print_unsigned(out, x, fmt);
}

namespace config {

template<>
void helper_t<char>::parse(in_t::ptr_t &ptr, char &val) {
	if((char)-1 < (char)1)
		parse_signed(ptr, val, NULL, &error);
	else
		parse_unsigned(ptr, val, NULL, &error);
}

template<>
void helper_t<char>::print(out_t &out, int, char const &val) {
	if((char)-1 < (char)1)
		print_signed(out, val);
	else
		print_unsigned(out, val);
}

template<>
void helper_t<char>::syntax(out_t &out) {
	out(CSTR("<char>"));
}

} // namespace config

#define __intdef(s, type) \
template<> \
bool in_t::helper_t<type>::parse( \
	ptr_t &ptr, type &val, char const *fmt, error_handler_t handler \
) { \
	return parse_##s(ptr, val, fmt, handler); \
} \
\
template<> \
void out_t::helper_t<type>::print( \
	out_t &out, type const &x, char const *fmt \
) { \
	print_##s(out, x, fmt); \
} \
\
namespace config { \
\
template<> \
void helper_t<type>::parse(in_t::ptr_t &ptr, type &val) { \
	parse_##s(ptr, val, NULL, &error); \
} \
\
template<> \
void helper_t<type>::print(out_t &out, int, type const &val) { \
	print_##s(out, val); \
} \
\
template<> \
void helper_t<type>::syntax(out_t &out) { \
	out(CSTR("<" #type ">")); \
} \
\
} // namespace config

#define __dintdef(type) \
	__intdef(signed, signed type) \
	__intdef(unsigned, unsigned type)

__dintdef(char)
__dintdef(short int)
__dintdef(int)
__dintdef(long int)
__dintdef(long long int)

#ifdef _LP64

typedef signed int int128_t __attribute__((mode(TI)));
typedef unsigned int uint128_t __attribute__((mode(TI)));

__intdef(signed, int128_t)
__intdef(unsigned, uint128_t)

#endif

#undef __dintdef
#undef __intdef

} // namespace pd
