// This file is part of the pd::base library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
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
	}

	if(s == sizeval_unlimited) {
		out(CSTR("unlimited"));
	} else if(s) {
		if(((s - (s % sizeval_exa)) >> prec) >= (s % sizeval_exa)) {
			out.print((uint64_t)(s / sizeval_exa))('E');
		}
		else if(((s - (s % sizeval_peta)) >> prec) >= (s % sizeval_peta)) {
			out.print((uint64_t)(s / sizeval_peta))('P');
		}
		else if(((s - (s % sizeval_tera)) >> prec) >= (s % sizeval_tera)) {
			out.print((uint64_t)(s / sizeval_tera))('T');
		}
		else if(((s - (s % sizeval_giga)) >> prec) >= (s % sizeval_giga)) {
			out.print((uint64_t)(s / sizeval_giga))('G');
		}
		else if(((s - (s % sizeval_mega)) >> prec) >= (s % sizeval_mega)) {
			out.print((uint64_t)(s / sizeval_mega))('M');
		}
		else if(((s - (s % sizeval_kilo)) >> prec) >= (s % sizeval_kilo)) {
			out.print((uint64_t)(s / sizeval_kilo))('K');
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
		sizeval = sizeval_unlimited;
	}
	else {
		helper_t<uint64_t>::parse(p, res);

		sizeval_t stmp;

		switch(p ? *p : '\0') {
			case 'E':
				stmp = res * sizeval_exa;
				if(stmp / sizeval_exa != res) goto over;
				++p;
				break;
			case 'P':
				stmp = res * sizeval_peta;
				if(stmp / sizeval_peta != res) goto over;
				++p;
				break;
			case 'T':
				stmp = res * sizeval_tera;
				if(stmp / sizeval_tera != res) goto over;
				++p;
				break;
			case 'G':
				stmp = res * sizeval_giga;
				if(stmp / sizeval_giga != res) goto over;
				++p;
				break;
			case 'M':
				stmp = res * sizeval_mega;
				if(stmp / sizeval_mega != res) goto over;
				++p;
				break;
			case 'K':
				stmp = res * sizeval_kilo;
				if(stmp / sizeval_kilo != res) goto over;
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
	out.print(sizeval);
}

template<>
void helper_t<sizeval_t>::syntax(out_t &out) {
	out(CSTR("<size>"));
}

} // namespace config

} // namespace pd
