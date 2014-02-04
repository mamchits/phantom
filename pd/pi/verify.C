// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

namespace pd {

struct pi_t::verify_t {
	pi_t const *origin;
	enum_table_t const &enum_table;

	inline verify_t(enum_table_t const &_enum_table) throw() :
		origin(NULL), enum_table(_enum_table) { }

	void do_verify(
		pi_t const &pi, unsigned int lev, pi_t const **bound
	);

	inline void __noreturn error(
		char const *msg, unsigned int lev,
		pi_t const *_obj, pi_t const *_req, pi_t const *_bound
	) {
		err_t err(err_t::_verify, str_t(msg, strlen(msg)));

		err.verify_lev() = lev;

		if(_obj)
			err.verify_obj() = _obj - origin;

		if(_req)
			err.verify_req() = _req - origin;

		if(_bound)
			err.verify_bound() = _bound - origin;

		throw exception_t(err);
	}

	inline root_t const *operator()(char const *buf, size_t size) {
		if(!buf)
			error("invalid call parameters", 0, NULL, NULL, NULL);

		if(size < sizeof(root_t::_size()))
			error("buffer too short", 0, NULL, NULL, NULL);

		root_t const *_root = (root_t const *)buf;
		if(_root->size * sizeof(pi_t) != size)
			error("wrong buffer size", 0, NULL, NULL, NULL);

		origin = (pi_t const *)_root;
		pi_t const *bound = origin + _root->size;

		do_verify(_root->value, 1, &bound);

		pi_t const *req = origin + root_t::_size();
		if(req != bound)
			error("junk between root and root data", 0, NULL, req, bound);

		return _root;
	}
};

void pi_t::verify_t::do_verify(
	pi_t const &pi, unsigned int lev, pi_t const **bound
) {
	c_place_t place(NULL);

	switch(pi.type()) {
		case _enum: {
			if(!enum_table.lookup(pi.__enum()))
				error("unknown enum", lev, &pi, NULL, NULL);

			return;
		}

		case _int29:
			return;

		case _uint64:
		case _float: {
			_size_t off = pi.offset();
			if(!off) error("wrong in-placed object", lev, &pi, NULL, NULL);
			place._pi = &pi + off;

			pi_t const *req = place._pi + _number_size;

			if(req != *bound)
				error("ill-placed number", lev, &pi, req, *bound);
		}
		break;

		case _string: {
			_size_t off = pi.offset();
			if(!off) return;
			place._pi = &pi + off;

			pi_t const *req = place._pi + string_t::_size(0);
			if(req > *bound)
				error("ill-placed string (var 1)", lev, &pi, req, *bound);

			req = place._pi + place.__string->_size();
			if(req != *bound)
				error("ill-placed string (var 2)", lev, &pi, req, *bound);

			char const *r = place.__raw + place.__string->_rsize();
			while(r < (char const *)req)
				if(*(r++))
					error("nonzero string padding", lev, &pi, req - 1, req);
		}
		break;

		case _array: {
			_size_t off = pi.offset();
			if(!off) return;
			place._pi = &pi + off;

			pi_t const *req = place._pi + array_t::_size(0);
			if(req > *bound)
				error("ill-placed array (var 1)", lev, &pi, req, *bound);

			req = place._pi + place.__array->_size();

			if(req > *bound)
				error("ill-placed array (var 2)", lev, &pi, req, *bound);

			for_each(ptr, *place.__array)
				do_verify(*ptr, lev + 1, bound);

			if(req != *bound)
				error("junk between array items and items data", lev, &pi, req, *bound);
		}
		break;

		case _map: {
			_size_t off = pi.offset();
			if(!off) return;
			place._pi = &pi + off;

			pi_t const *req = place._pi + map_t::_size(0);
			if(req > *bound)
				error("ill-placed map (var 1)", lev, &pi, req, *bound);

			req = place._pi + place.__map->_size();

			if(req > *bound)
				error("ill-placed map (var 2)", lev, &pi, req, *bound);

			for_each(ptr, *place.__map) {
				do_verify(ptr->key, lev + 1, bound);
				do_verify(ptr->value, lev + 1, bound);
			}

			if(req != *bound)
				error("junk between map items and items data", lev, &pi, req, *bound);

			if(!place.__map->index_verify())
				error("broken map index", lev, &pi, req - place.__map->index_size(), req);

			char const *r = place.__raw + place.__map->_rsize();
			while(r < (char const *)req)
				if(*(r++))
					error("nonzero map index padding", lev, &pi, req - 1, req);
		}
		break;

		default:
			error("ivalid object type", lev, &pi, NULL, NULL);
	}

	*bound = place._pi;
}

pi_t::root_t const *pi_t::verify(
	char const *buf, size_t size, enum_table_t const &enum_table
) {
	return verify_t(enum_table)(buf, size);
}

} // namespace pd
