// This file is part of the pd::base library.
// Copyright (C) 2013, 2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2013, 2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "stat.H"
#include "size.H"

namespace pd { namespace stat {

static sizeval_t calc_rate(sizeval_t val, interval_t interval, char &letter) {
	sizeval_t rate = 0;
	letter = '\0';

	if(!val)
		return rate;

	rate = val * interval::second / interval;
	if(rate) {
		letter = 's';
		return rate;
	}

	rate = val * interval::minute / interval;
	if(rate) {
		letter = 'm';
		return rate;
	}

	rate = val * interval::hour / interval;
	if(rate) {
		letter = 'h';
		return rate;
	}

	rate = val * interval::day / interval;
	if(rate) {
		letter = 'd';
		return rate;
	}

	rate = val * interval::week / interval;
	letter = 'w';
	return rate;
}

static string_t const count_stags[2] = {
	STRING("count"), STRING("rate")
};

template<>
void ctx_t::helper_t<count_t>::print(
	ctx_t &ctx, str_t const &_tag,
	count_t const &, count_t::res_t const &res
) {
	if(ctx.pre(_tag, NULL, 0, count_stags, 2)) {
		sizeval_t _val = res.val;

		if(ctx.format == ctx.json) {
			sizeval_t rate = res.val * interval::second / res.interval;
			ctx.out('[').print(_val)(',')(' ').print(rate)(']');
		}
		else if(ctx.format == ctx.html) {
			char rl = '\0';
			sizeval_t rate = calc_rate(_val, res.interval, rl);

			ctx.off();
			ctx.out
				(CSTR("<td>"))
				.print(_val, ".4")
				(CSTR("</td><td>"))
			;

			if(rl)
				ctx.out.print(rate, ".4")('/')(rl);
			else
				ctx.out(CSTR("&nbsp;"));

			ctx.out
				(CSTR("</td>"))
			;
		}
	}
}

static string_t const mmcount_stags[3] = {
	STRING("min"), STRING("avg"), STRING("max")
};

template<>
void ctx_t::helper_t<mmcount_t>::print(
	ctx_t &ctx, str_t const &_tag,
	mmcount_t const &, mmcount_t::res_t const &res
) {
	if(ctx.pre(_tag, NULL, 0, mmcount_stags, 3)) {
		if(res.min_val <= res.max_val) {
			sizeval_t avg = res.interval > interval::zero
				? res.tsum / res.interval
				: (res.min_val + res.max_val) / 2
			;

			if(ctx.format == ctx.json) {
				ctx.out
					('[').print(res.min_val)(',')(' ')
					.print(avg)(',')(' ')
					.print(res.max_val)(']')
				;
			}
			else if(ctx.format == ctx.html) {
				ctx.off();
				ctx.out
					(CSTR("<td>"))
					.print(res.min_val, ".4")
					(CSTR("</td><td>"))
					.print(avg, ".4")
					(CSTR("</td><td>"))
					.print(res.max_val, ".4")
					(CSTR("</td>"))
				;
			}
		}
		else {
			if(ctx.format == ctx.json)
				ctx.out(CSTR("[ ]"));
			else if(ctx.format == ctx.html) {
				ctx.off();
				ctx.out(CSTR("<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>"));
			}
		}
	}
}

static string_t const mminterval_stags[3] = {
	STRING("min"), STRING("avg"), STRING("max")
};

static void mminterval_print(
	ctx_t &ctx, str_t const &_tag,
	interval_t const &min_val, interval_t const &max_val,
	interval_sq_t const &tsum, interval_t const &interval
) {
	if(ctx.pre(_tag, NULL, 0, mminterval_stags, 3)) {
		if(min_val <= max_val) {
			interval_t avg = (min_val / 2 + max_val / 2);

			if(min_val.is_real() && max_val.is_real() && interval > interval::zero)
				avg = tsum / interval;

			if(ctx.format == ctx.json) {
				ctx.out
					('[').print(min_val / interval::microsecond)(',')(' ')
					.print(avg / interval::microsecond)(',')(' ')
					.print(max_val / interval::microsecond)(']')
				;
			}
			else if(ctx.format == ctx.html) {
				ctx.off();
				ctx.out
					(CSTR("<td>"))
					.print(min_val, ".2")
					(CSTR("</td><td>"))
					.print(avg, ".2")
					(CSTR("</td><td>"))
					.print(max_val, ".2")
					(CSTR("</td>"))
				;
			}
		}
		else {
			if(ctx.format == ctx.json)
				ctx.out(CSTR("[ ]"));
			else if(ctx.format == ctx.html) {
				ctx.off();
				ctx.out(CSTR("<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>"));
			}
		}
	}
}

template<>
void ctx_t::helper_t<mminterval_t>::print(
	ctx_t &ctx, str_t const &_tag,
	mminterval_t const &, mminterval_t::res_t const &res
) {
	mminterval_print(ctx, _tag, res.min_val, res.max_val, res.tsum, res.interval);
}

template<>
void ctx_t::helper_t<mmage_t>::print(
	ctx_t &ctx, str_t const &_tag,
	mmage_t const &, mmage_t::res_t const &res
) {
	mminterval_print(ctx, _tag, res.min_val, res.max_val, res.tsum, res.interval);
}

static string_t const vcount_stags[3] = {
	STRING("count"), STRING("rate"), STRING("%")
};

void vcount_print(
	ctx_t &ctx, str_t const &_tag,
	string_t const *tags, sizeval_t const *vals, size_t cnt,
	interval_t const &interval
) {
	if(ctx.pre(_tag, tags, cnt, vcount_stags, 3)) {
		if(ctx.format == ctx.json) {
			ctx.out('{');

			for(size_t i = 0; i < cnt; ++i) {
				sizeval_t _val = vals[i];
				sizeval_t rate = _val * interval::second / interval;

				if(i) ctx.out(',');
				ctx.out(' ')('\"')(tags[i])('\"')(' ')(':')(' ');

				ctx.out('[').print(_val)(',')(' ').print(rate)(']');
			}

			ctx.out(' ')('}');
		}
		else if(ctx.format == ctx.html) {
			sizeval_t sum = 0;
			for(size_t i = 0; i < cnt; ++i) sum += vals[i];

			ctx.off();
			for(size_t i = 0; i < cnt; ++i) {
				sizeval_t _val = vals[i];
				char rl = '\0';
				sizeval_t rate = calc_rate(_val, interval, rl);

				unsigned int prom = sum
					? (_val * 2000 + sum) / sum / 2
					: (i ? 0 : 1000)
				;

				ctx.out
					(CSTR("<td>"))
					.print(_val, ".4")
					(CSTR("</td><td>"))
				;

				if(rl)
					ctx.out.print(rate, ".4")('/')(rl);
				else
					ctx.out(CSTR("&nbsp;"));

				ctx.out
					(CSTR("</td><td>"))
				;

				if(sum)
					ctx.out.print(prom / 10)('.').print(prom % 10);
				else
					ctx.out(CSTR("&nbsp;"));

				ctx.out
					(CSTR("</td>"))
				;
			}
		}
	}
}

static string_t const tstate_stags[2] = {
	STRING("time"), STRING("%")
};

void tstate_print(
	ctx_t &ctx, str_t const &_tag,
	string_t const *tags, interval_t const *intervals, size_t cnt
) {
	if(ctx.pre(_tag, tags, cnt, tstate_stags, 2)) {
		if(ctx.format == ctx.json) {
			ctx.out('{');

			for(size_t i = 0; i < cnt; ++i) {
				if(i) ctx.out(',');
				ctx.out(' ')('\"')(tags[i])('\"')(' ')(':')(' ');

				ctx.out.print(intervals[i] / interval::microsecond);
			}

			ctx.out(' ')('}');
		}
		else if(ctx.format == ctx.html) {
			interval_t sum = interval::zero;
			for(size_t i = 0; i < cnt; ++i) sum += intervals[i];

			ctx.off();
			for(size_t i = 0; i < cnt; ++i) {
				interval_t interval = intervals[i];

				unsigned int prom = sum > interval::zero
					? (interval * 2000 + sum) / sum / 2
					: (i ? 0 : 1000)
				;

				ctx.out
					(CSTR("<td>"))
					.print(interval, ".2")
					(CSTR("</td><td>"))
				;

				if(sum > interval::zero)
					ctx.out.print(prom / 10)('.').print(prom % 10);
				else
					ctx.out(CSTR("&nbsp;"));

				ctx.out
					(CSTR("</td>"))
				;
			}
		}
	}
}

}} // namespace pd::stat
