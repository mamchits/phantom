// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "size.H"
#include "config_helper.H"

namespace pd {

template<>
void out_t::helper_t<sizeval_t>::print(
	out_t &out, sizeval_t const &s, char const *fmt
) {
	unsigned short int prec = 63;

	if(fmt) {
		if(*fmt == '.') {
			prec = 0;
			++fmt;
			while(*fmt >= '0' && *fmt <= '9') (prec *= 10) += (*(fmt++) - '0');
		}
		if(!prec) prec = 63;
	}
	else {
		out.print((uint64_t)s);
		return;
	}

	if(s == sizeval::unlimited) {
		out(CSTR("unlimited"));
	} else if(s) {
		if(((s - (s % sizeval::exa)) >> prec) >= (s % sizeval::exa)) {
			out.print((uint64_t)(s / sizeval::exa))('E');
		}
		else if(((s - (s % sizeval::peta)) >> prec) >= (s % sizeval::peta)) {
			out.print((uint64_t)(s / sizeval::peta))('P');
		}
		else if(((s - (s % sizeval::tera)) >> prec) >= (s % sizeval::tera)) {
			out.print((uint64_t)(s / sizeval::tera))('T');
		}
		else if(((s - (s % sizeval::giga)) >> prec) >= (s % sizeval::giga)) {
			out.print((uint64_t)(s / sizeval::giga))('G');
		}
		else if(((s - (s % sizeval::mega)) >> prec) >= (s % sizeval::mega)) {
			out.print((uint64_t)(s / sizeval::mega))('M');
		}
		else if(((s - (s % sizeval::kilo)) >> prec) >= (s % sizeval::kilo)) {
			out.print((uint64_t)(s / sizeval::kilo))('K');
		}
		else {
			out.print((uint64_t)s);
		}
	}
	else {
		out('0');
	}
}

namespace config {

template<>
void helper_t<sizeval_t>::parse(in_t::ptr_t &ptr, sizeval_t &sizeval) {
	in_t::ptr_t p = ptr;
	uint64_t res;

	if(p.match<ident_t>(CSTR("unlimited"))) {
		sizeval = sizeval::unlimited;
	}
	else {
		helper_t<uint64_t>::parse(p, res);

		sizeval_t stmp;

		switch(p ? *p : '\0') {
			case 'E':
				stmp = res * sizeval::exa;
				if(stmp / sizeval::exa != res) goto over;
				++p;
				break;
			case 'P':
				stmp = res * sizeval::peta;
				if(stmp / sizeval::peta != res) goto over;
				++p;
				break;
			case 'T':
				stmp = res * sizeval::tera;
				if(stmp / sizeval::tera != res) goto over;
				++p;
				break;
			case 'G':
				stmp = res * sizeval::giga;
				if(stmp / sizeval::giga != res) goto over;
				++p;
				break;
			case 'M':
				stmp = res * sizeval::mega;
				if(stmp / sizeval::mega != res) goto over;
				++p;
				break;
			case 'K':
				stmp = res * sizeval::kilo;
				if(stmp / sizeval::kilo != res) goto over;
				++p;
				break;
			default:
				stmp = res;
		}

		sizeval = stmp;
	}

	ptr = p;
	return;

over:
	error(ptr, "integer overflow");
}

template<>
void helper_t<sizeval_t>::print(out_t &out, int, sizeval_t const &sizeval) {
	out.print(sizeval, ".");
}

template<>
void helper_t<sizeval_t>::syntax(out_t &out) {
	out(CSTR("<size>"));
}

} // namespace config

} // namespace pd
