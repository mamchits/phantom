// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

#include <pd/base/out.H>

#include <stdlib.h>
#include <stdio.h> // snprintf float

namespace pd {

class pi_t::print_t {
	out_t &out;
	unsigned int max_inline;
	enum_table_t const &enum_table;

	void do_print(pi_t const &pi, unsigned int depth);

	unsigned int check_volume(pi_t const &pi) const {
		switch(pi.type()) {
			case _enum:
			case _int29:
				return 1;

			case _uint64:
			case _float:
				return 2;

			case _string: {
				string_t const &string = pi.__string();
				return string._size();
			}
			case _array: {
				unsigned int v = 0;
				for_each(ptr, pi.__array()) {
					v += check_volume(*ptr);
					if(v > max_inline) break;
				}

				return v;
			}
			case _map: {
				unsigned int v = 0;
				for_each(ptr, pi.__map()) {
					v += check_volume(ptr->key);
					if(v > max_inline) break;

					v += check_volume(ptr->value);
					if(v > max_inline) break;
				}

				return v;
			}
			default:
				abort();
		}
	}

public:
	inline print_t(
		out_t &_out, unsigned int _max_inline, enum_table_t const &_enum_table
	) throw() :
		out(_out), max_inline(_max_inline), enum_table(_enum_table) { }

	inline ~print_t() throw() { }

	inline void operator()(pi_t const &pi) { do_print(pi, 0); }
};

static inline void indent(out_t &out, int depth, bool t) {
	if(t) {
		out('\n'); for(int i = 0; i < depth; ++i) out('\t');
	}
	else
		out(' ');
}

void pi_t::print_t::do_print(pi_t const &pi, unsigned int depth) {
	switch(pi.type()) {
		case _enum: {
			str_t const str = enum_table.lookup(pi.__enum());
			if(str)
				out(str);
			else
				out(CSTR("unknown_")).print(pi.__enum(), "08X");
		}
		break;

		case _int29: out.print(pi.__int29()); break;

		case _uint64: out.print(pi.__uint64()); break;

		case _float: {
			size_t const _size = 128;
			char buf[_size + 1];
			size_t size = snprintf(buf, _size, "%e", pi.__float());
			minimize(size, _size);
			buf[_size] = '\0';
			out(str_t(buf, size));
		}
		break;

		case _string: out.print(pi.__string().str(), "e8"); break;

		case _array: {
			bool t = max_inline
				? max_inline == ~0U ? false : check_volume(pi) > max_inline
				: true
			;

			out('[');

			bool comma = false;
			for_each(ptr, pi.__array()) {
				if(comma) out(',');
				indent(out, depth + 1, t);
				do_print(*ptr, depth + 1);
				comma = true;
			}

			indent(out, depth, t);
			out(']');
		}
		break;

		case _map: {
			bool t = max_inline
				? max_inline == ~0U ? false : check_volume(pi) > max_inline
				: true
			;

			out('{');

			bool comma = false;
			for_each(ptr, pi.__map()) {
				if(comma) out(',');
				indent(out, depth + 1, t);
				do_print(ptr->key, depth + 1);
				out(':')(' ');
				do_print(ptr->value, depth + 1);
				comma = true;
			}

			indent(out, depth, t);
			out('}');
		}
		break;

		default:;
			abort();
	}
}

void pi_t::print(
	out_t &out, unsigned int max_inline, enum_table_t const &enum_table
) const {
	 pi_t::print_t(out, max_inline, enum_table)(*this);
}

void pi_t::print_text(
	out_t &out, root_t const *root, enum_table_t const &enum_table
) {
	root->value.print(out, 10, enum_table);
}

void pi_t::print_app(
	out_t &out, root_t const *root, enum_table_t const &
) {
	out(str_t((char const *)root, root->size * sizeof(_size_t)));
}

template<>
void out_t::helper_t<pi_t>::print(
	out_t &out, pi_t const &pi, char const *fmt
) {
	unsigned int max_inline = ~0U;
	if(fmt) {
		max_inline = 0;
		while(*fmt >= '0' && *fmt <= '9') (max_inline *= 10) += (*(fmt++) - '0');
	}

	pi_t::print_t(out, max_inline, pi_t::enum_table_def)(pi);
}

} // namespace pd
