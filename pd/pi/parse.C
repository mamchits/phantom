// This file is part of the pd::pi library.
// Copyright (C) 2012-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2012-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "pi.H"

#include <pd/base/size.H>

#include <errno.h> // Really dont needed

namespace pd {

struct pi_t::parse_t {
	in_t::ptr_t &__ptr;
	mem_t const &mem;
	enum_table_t const &enum_table;

	in_t::ptr_t ptr;

	enum item_type_t {
		_item_ctx, _item_ctx_ptr, _item_string, _item_number, _item_enum
	};

	struct ctx_t {
		item_type_t const item_type;

		ctx_t *next;
		enum type_t { _root = 0, _array, _map } type;
		enum state_t { _init = 0, _item, _comma, _key, _colon } state;
		_size_t count;

		pi_t *ref;

		inline void setup(type_t _type, parse_t &parse) throw() {
			assert(item_type == _item_ctx);

			next = parse.ctx;
			parse.ctx = this;

			if((type = _type) != ctx_t::_root)
				++parse.ptr;

			state = _init;
			count = 0;
			ref = NULL;
		}

		void __noreturn error(parse_t &parse);
		void item(parse_t &parse);
		void comma(parse_t &parse);
		void colon(parse_t &parse);
		void end_of_array(parse_t &parse);
		void end_of_map(parse_t &parse);
		void end_of_root(parse_t &parse);
	};

	struct ctx_ptr_t {
		item_type_t const item_type;

		ctx_t *ctx;

		inline void setup(parse_t &parse) {
			assert(item_type == _item_ctx_ptr);

			++parse.ptr;

			ctx = parse.ctx;

			if(ctx->count) {
				switch(ctx->type) {
					case ctx_t::_array:
						parse.size += array_t::_size(ctx->count);
					break;
					case ctx_t::_map:
						assert(!(ctx->count % 2));
						parse.size += map_t::_size(ctx->count / 2);
					break;
					default:
						abort();
				}
			}

			++(parse.ctx = ctx->next)->count;
		}
	};

	struct string_val_t {
		item_type_t const item_type;

		in_t::ptr_t origin;
		size_t len;
		bool has_esc;

		void setup(parse_t &parse);

		void put(char *dst);
	};

	struct number_val_t {
		item_type_t const item_type;

		type_t type;
		union { int64_t i; double f; };

		void setup(parse_t &parse);
	};

	struct enum_val_t {
		item_type_t const item_type;
		unsigned int val;

		void setup(parse_t &parse);
	};

	static inline size_t item_size(item_type_t item_type) {
		switch(item_type) {
			case _item_ctx: return sizeof(ctx_t);
			case _item_ctx_ptr: return sizeof(ctx_ptr_t);
			case _item_string: return sizeof(string_val_t);
			case _item_number: return sizeof(number_val_t);
			case _item_enum: return sizeof(enum_val_t);
		}
		abort();
	}

	union item_t {
		item_type_t item_type;
		ctx_t ctx;
		ctx_ptr_t ctx_ptr;
		string_val_t string_val;
		number_val_t number_val;
		enum_val_t enum_val;
	};

	struct pool_t {
		struct chunk_t {
			chunk_t *next;
			static size_t const size = 16 * sizeval::kilo;
			char buf[size];
			size_t idx;

			inline chunk_t(chunk_t *_next) : next(_next), idx(size) { }
			inline item_t *alloc(item_type_t item_type) {
				size_t _size = item_size(item_type);

				if(idx > _size) {
					idx -= _size;
					item_t *res = (item_t *)&buf[idx];
					res->item_type = item_type;
					return res;
				}

				return NULL;
			}

			inline operator bool() const { return idx < size; }

			inline item_t *pop() {
				item_t *res = (item_t *)&buf[idx];

				size_t _size = item_size(res->item_type);

				idx += _size;
				assert(idx <= size);
				return res;
			}
		};

		chunk_t *chunk;

		inline pool_t() throw() : chunk(new chunk_t(NULL)) { }

		inline item_t *alloc(item_type_t item_type) {
			item_t *res = chunk->alloc(item_type);

			return res ?: (chunk = new chunk_t(chunk))->alloc(item_type);
		}

		inline item_t *pop() {
			if(!*chunk) {
				chunk_t *tmp = chunk;
				chunk = chunk->next;
				delete tmp;
			}

			return chunk->pop();
		}

		inline ~pool_t() throw() {
			while(chunk) {
				chunk_t *tmp = chunk;
				chunk = chunk->next;
				delete tmp;
			}
		}
	};

	pool_t pool;

	ctx_t *ctx;
	_size_t size;
	place_t place;

	inline parse_t(
		in_t::ptr_t &_ptr, mem_t const &_mem, enum_table_t const &_enum_table
	) throw() :
		__ptr(_ptr), mem(_mem), enum_table(_enum_table),
		ptr(__ptr), ctx(NULL), size(root_t::_size()), place(NULL) { }

	root_t *dump();

	root_t *operator()();

	inline void __noreturn error(char const *msg) {
		in_t::ptr_t _ptr = __ptr;
		size_t limit = ptr - __ptr;

		size_t lineno = 1;
		while(_ptr.scan("\n", 1, limit)) { ++_ptr; ++lineno; --limit; }

		err_t err(err_t::_parse, str_t(msg, strlen(msg)));
		err.parse_lineno() = lineno;
		err.parse_pos() = ptr - _ptr + 1;
		err.parse_abspos() = ptr - __ptr;

		throw exception_t(err);
	}
};

void __noreturn pi_t::parse_t::ctx_t::error(parse_t &parse) {
	switch(type) {
		case _root:
			switch(state) {
				case _init:
					parse.error("root object expected");
				case _item:
					parse.error("junk behind the root object");
				default:;
			}
			break;
		case _array:
			switch(state) {
				case _init:
					parse.error("object or end of array expected");
				case _item:
					parse.error("comma or end of array expected");
				case _comma:
					parse.error("object expected");
				default:;
			}
			break;
		case _map:
			switch(state) {
				case _init:
					parse.error("key or end of map expected");
				case _key:
					parse.error("colon expected");
				case _colon:
					parse.error("object expected");
				case _item:
					parse.error("comma or end of map expected");
				case _comma:
					parse.error("key expected");
				default:;
			}
			break;
	}

	abort();
}

void pi_t::parse_t::ctx_t::item(parse_t &parse) {
	switch(type) {
		case _root:
			if(state == _init) {
				state = _item; return;
			}
			break;
		case _array:
			if(state == _init || state == _comma) {
				state = _item; return;
			}
			break;
		case _map:
			if(state == _colon) {
				state = _item; return;
			}
			else if(state == _init || state == _comma) {
				state = _key; return;
			}
			break;
	}

	error(parse);
}

void pi_t::parse_t::ctx_t::comma(parse_t &parse) {
	if(state == _item) {
		state = _comma;
		return;
	}

	error(parse);
}

void pi_t::parse_t::ctx_t::colon(parse_t &parse) {
	if(type == _map && state == _key) {
		state = _colon;
		return;
	}

	error(parse);
}

void pi_t::parse_t::ctx_t::end_of_array(parse_t &parse) {
	if(type == _array && (state == _init || state == _item))
		return;

	error(parse);
}

void pi_t::parse_t::ctx_t::end_of_map(parse_t &parse) {
	if(type == _map && (state == _init || state == _item))
		return;

	error(parse);
}

void pi_t::parse_t::ctx_t::end_of_root(parse_t &parse) {
	if(type == _root && state == _item)
		return;

	error(parse);
}

static inline bool parse_oct(char c, char &res) {
	switch(c) {
		case '0'...'7': res = res * 8 + (c - '0'); return true;
	}
	return false;
}

static inline bool parse_hex(char c, unsigned int &res) {
	switch(c) {
		case '0'...'9': res = res * 16 + (c - '0'); return true;
		case 'a'...'f': res = res * 16 + 10 + (c - 'a'); return true;
		case 'A'...'F': res = res * 16 + 10 + (c - 'A'); return true;
	}
	return false;
}

void pi_t::parse_t::string_val_t::setup(parse_t &parse) {
	assert(item_type == _item_string);

	in_t::ptr_t &ptr = parse.ptr;
	++ptr;

	origin = ptr;

	len = 0;
	has_esc = false;

	char c;

	while(ptr) {
		switch(*ptr) {
			case '\\': {
				has_esc = true;
				if(!++ptr)
					goto out;

				switch(c = *ptr) {
					case 'r': case 'n': case 't': case 'v':
					case 'b': case 'f': case 'a': case 'e':
					case '"': case '\\': case '/': break;

					case '0' ... '3': {
						c -= '0';
						if(!++ptr)
							goto out;
						if(!parse_oct(*ptr, c))
							parse.error("not an octal digit");
						if(!++ptr)
							goto out;
						if(!parse_oct(*ptr, c))
							parse.error("not an octal digit");
					}
					break;

					case 'u': {
						unsigned int u = 0;
						for(unsigned int q = 0; q < 4; ++q) {
							if(!++ptr)
								goto out;
							if(!parse_hex(*ptr, u))
								parse.error("not an hex digit");
						}
						if(u >= 0x80) ++len;
						if(u >= 0x800) ++len;
					}
					break;

					default:
						parse.error("illegal escape character");
				}
			}
			break;

			case '"':
				++ptr;

				if(len)
					parse.size += string_t::_size(len);

				++parse.ctx->count;

				return;

			case '\0' ... (' ' - 1): case '\177':
				parse.error("control character in the string");

		}
		++ptr;
		++len;
	}

out:
	parse.error("unexpected end of string");
}

void pi_t::parse_t::string_val_t::put(char *dst) {
	if(!has_esc) {
		out_t out(dst, len);
		out(origin, len);
	}
	else {
		char c;
		in_t::ptr_t ptr = origin;
		while(true) {
			switch(c = *ptr) {
				case '\\': {
					switch(c = *++ptr) {
						case 'r': c = '\r'; break;
						case 'n': c = '\n'; break;
						case 't': c = '\t'; break;
						case 'v': c = '\v'; break;
						case 'b': c = '\b'; break;
						case 'f': c = '\f'; break;
						case 'a': c = '\a'; break;
						case 'e': c = '\e'; break;
						case '"': case '\\': case '/': break;

						case '0' ... '3': {
							c -= '0';
							if(!parse_oct(*++ptr, c) || !parse_oct(*++ptr, c))
								abort();
						}
						break;

						case 'u': {
							unsigned int u = 0;
							for(unsigned int q = 0; q < 4; ++q) {
								if(!parse_hex(*++ptr, u))
									abort();
							}

							if(u < 0x80) {
								*(dst++) = u;
							}
							else if(u < 0x800) {
								*(dst++) = 0xc0 | (u >> 6);
								*(dst++) = 0x80 | (u & 0x3f);
							}
							else {
								*(dst++) = 0xe0 | (u >> 12);
								*(dst++) = 0x80 | ((u >> 6) & 0x3f);
								*(dst++) = 0x80 | (u & 0x3f);
							}

							++ptr;
							continue;
						}
						break;

						default:
							abort();
					}
				}
				break;

				case '"':
					return;

				case '\0' ... (' ' - 1): case '\177':
					abort();
			}
			++ptr;
			*(dst++) = c;
		}
	}
}

void pi_t::parse_t::number_val_t::setup(parse_t &parse) {
	assert(item_type == _item_number);

	int neg = 0;

	in_t::ptr_t &ptr = parse.ptr;
	in_t::ptr_t _ptr = ptr;
	char c = *ptr;

	if(c == '-') {
		neg = 1;
		++ptr;

		if(!ptr)
			parse.error("digit expected");

		c = *ptr;

		if(c < '0' || c > '9')
			parse.error("digit expected");
	}

	uint64_t _val = c - '0';

	while(++ptr) {
		c = *ptr;

		if(c >= '0' && c <= '9') {
			uint64_t tmp = _val * 10 + (c - '0');
			if(tmp / 10 != _val) {
				ptr = _ptr;
				parse.error("integer overflow");
			}

			_val = tmp;
		}
		else if(c == '.' || c == 'e' || c =='E') {
			while(++ptr) {
				switch(*ptr) {
					case '.': case 'e': case 'E': case '+': case '-': case '0' ... '9':
						continue;
				}
				break;
			}

			size_t size = ptr - _ptr;
			char buf[size + 1];
			{ out_t out(buf, size + 1); out(_ptr, size)('\0'); }

			char *e = buf;
			int _errno = errno;

			errno = 0;

			// FIXME: Use sane parsing function here.
			f = strtod(buf, &e);
			if(errno || e != &buf[size]) {
				ptr = _ptr;
				parse.error("floating point number parsing error");
			}

			errno = _errno;

			type = _float;

			parse.size += _number_size;
			++parse.ctx->count;

			return;
		}
		else {
			break;
		}
	}

	if(neg) {
		if(_val > ((uint64_t)1 << 63)) {
			ptr = _ptr;
			parse.error("integer overflow");
		}

		i = -_val;
		type = i >= _int29_min ? _int29 : _uint64;
	}
	else {
		type = _val <= _int29_max ? _int29 : _uint64;
		i = _val;
	}

	if(type != _int29)
		parse.size += _number_size;

	++parse.ctx->count;

	return;
}

void pi_t::parse_t::enum_val_t::setup(parse_t &parse) {
	assert(item_type == _item_enum);

	in_t::ptr_t &ptr = parse.ptr;
	in_t::ptr_t _ptr = ptr;

	while(++ptr) {
		switch(*ptr) {
			case 'a' ... 'z': case 'A' ... 'Z': case '_': case '0' ... '9': continue;
		}
		break;
	}

	pd::string_t string(_ptr, ptr - _ptr);

	val = parse.enum_table.lookup(string.str());

	if(val == ~0U) {
		ptr = _ptr;
		parse.error("unknown enum value");
	}

	++parse.ctx->count;

	return;
}

static inline bool is_space(char c) {
	switch(c) {
		case ' ': case'\t': case '\n': case '\r': return true;
	}

	return false;
}

static inline bool skip_space(in_t::ptr_t &ptr, char &c) {
	while(ptr) {
		char _c;

		if(!is_space(_c = *ptr)) {
			c = _c;
			return true;
		}

		++ptr;
	}
	return false;
}

pi_t::root_t *pi_t::parse_t::operator()() {
	pool.alloc(_item_ctx)->ctx.setup(ctx_t::_root, *this);

	char c;

	while(skip_space(ptr, c)) {
		switch(c) {
			case '[': {
				ctx->item(*this);
				pool.alloc(_item_ctx)->ctx.setup(ctx_t::_array, *this);
			}
			break;
			case '{': {
				ctx->item(*this);
				pool.alloc(_item_ctx)->ctx.setup(ctx_t::_map, *this);
			}
			break;
			case ',': {
				ctx->comma(*this);
				++ptr;
			}
			break;
			case ':': {
				ctx->colon(*this);
				++ptr;
			}
			break;
			case ']': {
				ctx->end_of_array(*this);
				pool.alloc(_item_ctx_ptr)->ctx_ptr.setup(*this);
			}
			break;
			case '}': {
				ctx->end_of_map(*this);
				pool.alloc(_item_ctx_ptr)->ctx_ptr.setup(*this);
			}
			break;
			case '"': {
				ctx->item(*this);
				pool.alloc(_item_string)->string_val.setup(*this);
			}
			break;
			case '0' ... '9': case '-': {
				ctx->item(*this);
				pool.alloc(_item_number)->number_val.setup(*this);
			}
			break;
			case 'a' ... 'z': case 'A' ... 'Z': case '_': {
				ctx->item(*this);
				pool.alloc(_item_enum)->enum_val.setup(*this);
			}
			break;
			case ';': {
				++ptr;
				goto out;
			}
			break;
			default:
				ctx->error(*this);
		}
	}

out:
	ctx->end_of_root(*this);
	return dump();
}

pi_t::root_t *pi_t::parse_t::dump() {
	pi_t::root_t *res_root = root_t::__new(size, mem);
	ctx->ref = &res_root->value + 1;
	place.__root = res_root;

	place._pi += place.__root->_size();

	while(true) {
		item_t *item = pool.pop();
		switch(item->item_type) {
			case _item_ctx: {
				ctx_t &_ctx = item->ctx;

				assert(ctx == &_ctx);

				switch(ctx->type) {
					case ctx_t::_array: {
						assert(!_ctx.count || (((array_t *)_ctx.ref) - 1)->count == _ctx.count);
						ctx = _ctx.next;
					}
					break;
					case ctx_t::_map: {
						if(_ctx.count) {
							map_t *map = ((map_t *)_ctx.ref) - 1;
							assert(map->count == _ctx.count / 2);
							map->index_make();
						}
						ctx = _ctx.next;
					}
					break;
					case ctx_t::_root: {
						assert(place._pi == (pi_t *)res_root + size);
						__ptr = ptr;
						return res_root;
					}
					default:
						abort();
				}
			}
			break;
			case _item_ctx_ptr: {
				ctx_ptr_t &ctx_ptr = item->ctx_ptr;

				assert(ctx == ctx_ptr.ctx->next);

				pi_t *ref = --ctx->ref;
				ctx = ctx_ptr.ctx;

				switch(ctx->type) {
					case ctx_t::_array: {
						if(ctx->count) {
							count_t count = ctx->count;
							place.__array->count = count;
							ctx->ref = &place.__array->items[count];

							ref->setup(_array, place._pi);
							place._pi += place.__array->_size();
						}
						else
							ref->setup(_array);
					}
					break;
					case ctx_t::_map: {
						if(ctx->count) {
							count_t count = ctx->count / 2;
							place.__map->count = count;
							ctx->ref = &place.__map->items[count].key;

							ref->setup(_map, place._pi);
							place._pi += place.__map->_size();
						}
						else
							ref->setup(_map);
					}
					break;
					default:
						abort();
				}
			}
			break;
			case _item_string: {
				string_val_t &string_val = item->string_val;

				pi_t *ref = --ctx->ref;
				if(string_val.len) {
					place.__string->count = string_val.len;
					_size_t size = place.__string->_size();
					place._pi[size - 1].value = 0U;

					string_val.put(place.__string->items);

					ref->setup(_string, place._pi);
					place._pi += size;
				}
				else
					ref->setup(_string);
			}
			break;
			case _item_number: {
				number_val_t &number_val = item->number_val;

				pi_t *ref = --ctx->ref;
				switch(number_val.type) {
					case _float: {
						*place.__float = number_val.f;

						ref->setup(_float, place._pi);
						place._pi += _number_size;
					}
					break;
					case _uint64: {
						*place.__uint64 = number_val.i;

						ref->setup(_uint64, place._pi);
						place._pi += _number_size;
					}
					break;
					case _int29: {
						ref->setup(_int29, number_val.i);
					}
					break;
					default:
						abort();
				}
			}
			break;
			case _item_enum: {
				enum_val_t &enum_val = item->enum_val;

				pi_t *ref = --ctx->ref;
				ref->setup(_enum, enum_val.val);
			}
			break;
			default:
				abort();
		}
	}
}

pi_t::root_t *pi_t::parse_text(
	in_t::ptr_t &_ptr, mem_t const &mem, enum_table_t const &enum_table
) {
	return parse_t(_ptr, mem, enum_table)();
}

pi_t::root_t *pi_t::parse_app(
	in_t::ptr_t &_ptr, mem_t const &mem, enum_table_t const &enum_table
) {
	in_t::ptr_t ptr = _ptr;
	_size_t _size = 0;
	for(int i = 0; i < 32; i += 8)
		_size |= (*(ptr++) << i);

	size_t size = _size * sizeof(pi_t);
	char *buf = (char *)mem.alloc(size);

	try {
		{ out_t out(buf, size); out(_ptr, size); }

		verify(buf, size, enum_table);

		_ptr += size;

		return (root_t *)buf;
	}
	catch(...) {
		mem.free(buf);
		throw;
	}
}

} // namespace pd
