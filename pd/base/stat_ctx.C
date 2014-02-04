// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "stat_ctx.H"
#include "exception.H"

namespace pd { namespace stat {

static __thread ctx_t *ctx_current = NULL;

static ctx_t *&default_current_funct() throw() { return ctx_current; }

static ctx_t::current_funct_t current_funct = &default_current_funct;

ctx_t *&ctx_t::current() throw() {
	try {
		return (*current_funct)();
	}
	catch(...) { }

	return default_current_funct();
}

static ctx_t *reg(ctx_t *new_ctx, bool expect_null) {
	ctx_t *&ctx = ctx_t::current();
	ctx_t *res = ctx;

	if((res == NULL) != expect_null)
		throw exception_log_t(log::error, "Wrong stat::ctx_t creation");

	ctx = new_ctx;
	return res;
}

ctx_t::current_funct_t ctx_t::setup(current_funct_t _current_funct) throw() {
	current_funct_t res = current_funct;
	current_funct = _current_funct ?: &default_current_funct;
	return res;
}

void ctx_t::off(unsigned int j) const {
	out.lf();
	for(size_t i = 0; i < lev + j; ++i)
		out(' ')(' ');
}

void ctx_t::json_tag(str_t const &name, unsigned int j) {
	if(count++) out(',');
	off(j);
	out('\"')(name)('\"')(' ')(':')(' ');
}

ctx_t::ctx_t(
	out_t &_out, format_t _format, size_t _res_no, bool _clear,
	unsigned int _lev
) :
	prev(reg(this, true)),
	out(_out), format(_format), variant(0), size(1),
	res_no(_res_no), clear(_clear), tag(CSTR("")), lev(_lev), count(0), flag(false) {

	if(format == json)
		out('{');
}

ctx_t::ctx_t(
	out_t &_out, format_t _format, size_t _res_no, bool _clear,
	str_t const &_tag, bool used, unsigned int _lev
) :
	prev(reg(this, true)),
	out(_out), format(_format), variant(0), size(1),
	res_no(_res_no), clear(_clear), tag(_tag), lev(_lev), count(0), flag(false) {

	if(format == json) {
		if(used) out(',').lf();
		out('\"')(tag)('\"')(' ')(':')(' ')('{');
	}
}

ctx_t::ctx_t(str_t const &_tag, int _variant, size_t _size) :
	prev(reg(this, false)),
	out(prev->out), format(prev->format), variant(_variant), size(_size),
	res_no(prev->res_no), clear(prev->clear), tag(_tag),
	lev(prev->lev + 1), count(0), flag(false) {

	if(format == json) {
		prev->json_tag(tag);
		out('{');
	}
	else if(format == html) {
		if(!variant && (prev->variant == 1 || prev->variant == 2)) variant = 2;

		if(variant == -1) {
			lev = prev->lev;
			off();
			out(CSTR("<hr><h3 align=left>"))(tag)(CSTR("</h3>"));
		}
		else if(variant == 1) {
			lev = prev->lev;
			off();
			out(CSTR("<table border class=\"stat\">"));
		}
		else if(prev->variant != 1) {
			lev = prev->lev;
		}
	}
}

ctx_t::~ctx_t() throw() {
	try {
		if(format == json) {
			if(prev) {
				if(count) prev->off();
				else out(' ');
				out('}');
			}
			else {
				out.lf()('}');
			}
		}
		else if(format == html) {
			if(variant == 1) {
				off();
				out(CSTR("</table><br>"));
			}
			else if(variant >= 2 && flag) {
				--lev;
				off();
				out(CSTR("</tr>"));
			}
		}
	}
	catch(...) { }

	current() = prev;
}

static void print_tag(ctx_t &ctx, size_t rowspan) {
	rowspan *= ctx.size;

	if(ctx.prev->variant == 2 && !ctx.prev->count++)
		print_tag(*ctx.prev, rowspan);

	ctx.out(CSTR("<th"));
	if(rowspan > 1)
		ctx.out(CSTR(" rowspan=")).print(rowspan);

	ctx.out('>');

	if(ctx.tag.size() > 8 && ctx.tag.size() < rowspan * 2)
		ctx.out(CSTR("<div class=\"bigtag\">"))(ctx.tag)(CSTR("</div>"));
	else
		ctx.out(ctx.tag ?: CSTR("&gt;"));

	ctx.out(CSTR("</th>"));
}

bool ctx_t::pre(
	str_t const &_tag,
	string_t const *tags, size_t cnt,
	string_t const *stags, size_t scnt
) {
	if(format == json) {
		json_tag(_tag);
		return true;
	}

	if(format != html)
		return false;

	if(variant == 2) {
		if(!count++) {
			off();
			out(CSTR("<tr>"));
			++lev;
			flag = true;

			if(!(prev->variant == 1 && prev->flag)) {
				off();
				print_tag(*this, 1);
			}
		}
		return true;
	}
	else if(variant == 3) {
		if(!count++) {
			off();
			out(CSTR("<tr>"));
			++lev;
			flag = true;

			if(!(prev->prev->variant == 1 && prev->prev->flag)) {
				size_t depth;
				str_t _tag = prev->table_root(&depth).tag;

				off();
				out(CSTR("<th rowspan=3"));

				if(depth > 1)
					out(CSTR(" colspan=")).print(depth);

				out('>')(_tag ?: CSTR("&nbsp;"))(CSTR("</th>"));
			}
		}

		off();

		if(tags) {
			if(stags) {
				out(CSTR("<th colspan=")).print(cnt * scnt)(CSTR(">"));
			}
			else {
				out(CSTR("<th colspan=")).print(cnt)(CSTR(">"));
			}
		}
		else {
			if(stags) {
				out(CSTR("<th rowspan=2 colspan=")).print(scnt)(CSTR(">"));
			}
			else {
				out(CSTR("<th rowspan=3>"));
			}
		}

		out(_tag)(CSTR("</th>"));
	}
	else if(variant == 4) {
		if(!count++) {
			off();
			out(CSTR("<tr>"));
			++lev;
			flag = true;
		}
		if(tags) {
			if(stags) {
				off();
				for(size_t i = 0; i < cnt; ++i)
					out(CSTR("<th colspan=")).print(scnt)(CSTR(">"))(tags[i])(CSTR("</th>"));
			}
			else {
				off();
				for(size_t i = 0; i < cnt; ++i)
					out(CSTR("<th rowspan=2>"))(tags[i])(CSTR("</th>"));
			}
		}
	}
	else if(variant == 5) {
		if(!count++) {
			off();
			out(CSTR("<tr>"));
			++lev;
			flag = true;
		}
		if(stags) {
			if(!tags) cnt = 1;
			off();
			for(size_t i = 0; i < cnt; ++i)
				for(size_t j = 0; j < scnt; ++j)
					out(CSTR("<th>"))(stags[j])(CSTR("</th>"));
		}
	}

	return false;
}

void link(string_t const &master) {
	ctx_t *ctx = ctx_t::current();

	if(!ctx)
		return;

	if(ctx->format == ctx->json) {
		ctx->json_tag(CSTR("link"));
		ctx->out('\"')(master.str())('\"');
	}
}

size_t res_count = 0;

}} // namespace pd::stat
