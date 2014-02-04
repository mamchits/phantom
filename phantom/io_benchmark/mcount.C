// This file is part of the phantom::io_benchmark module.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "mcount.H"

namespace phantom { namespace io_benchmark {

void mcount_t::res_t::print(
	stat::ctx_t &ctx, str_t const &_tag, tags_t const &tags
) {
	if(ctx.format == ctx.json) {
		ctx.json_tag(_tag); ctx.out('{');

		size_t cnt = tags.size();

		bool first = true;
		for(size_t i = 0; i < cnt; ++i) {
			if(!vals[i])
				continue;

			if(!first) ctx.out(',');
			first = false;
			ctx.out(' ')('\"'); tags.print(ctx.out, i); ctx.out('\"')(' ')(':')(' ');
			ctx.out.print(vals[i]);
		}

		ctx.out(' ')('}');
	}
	else if(ctx.format == ctx.html) {
		ctx.off();
		ctx.out(CSTR("<table border=1 class=\"stat\">"));
		ctx.off(1);
		ctx.out
			(CSTR("<tr><th>"))(ctx.tag)
			(CSTR("</th><th>count</th><th>%</th></tr>"))
		;

		size_t cnt = tags.size();

		uint64_t sum = 0;
		for(size_t i = 0; i < cnt; ++i)
			sum += vals[i];

		for(size_t i = 0; i < cnt; ++i) {
			uint64_t val = vals[i];
			if(!val)
				continue;

			unsigned int prom = (val * 2000 + sum) / sum / 2;

			ctx.off(1);
			ctx.out(CSTR("<tr><td>")); tags.print(ctx.out, i);
			ctx.out
				(CSTR("</td><td>")).print(vals[i])
				(CSTR("</td><td>")).print(prom / 10)('.').print(prom % 10)
				(CSTR("</td></tr>"))
			;
		}

		ctx.off(1);
		ctx.out
			(CSTR("<tr><th>total</th><td>"))
			.print(sum)(CSTR("</td><td>&nbsp;</td></tr>"))
		;

		ctx.off(); ctx.out(CSTR("</table><br>"));
	}
}

}} // namespace phantom::io_benchmark
