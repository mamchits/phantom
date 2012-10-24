// This file is part of the pd::bq library.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_heap.H"

#include <pd/base/exception.H>

#include <malloc.h>

namespace pd {

void bq_heap_t::place(item_t *item, size_t i, bool flag) throw() {
	size_t j;

	assert(i > 0);

	for(; (j = i / 2); i = j) {
		item_t *_item = get(j);

		if(hcmp(_item, item)) break;

		put(i, _item);

		flag = false;
	}

	if(flag) {
		for(; (j = i * 2) <= count; i = j) {
			item_t *_item = get(j);

			if(j < count) {
				item_t *__item = get(j + 1);
				if(!hcmp(_item, __item)) {
					++j;
					_item = __item;
				}
			}

			if(hcmp(item, _item)) break;

			put(i, _item);
		}
	}

	put(i, item);
}

void bq_heap_t::insert(item_t *item) throw() {
	assert(!item->ind);
	assert(!item->heap);

	if(count >= maxcount) {
		maxcount *= 2;
		if(maxcount < 128) maxcount = 128;

		item_t **_items = (item_t **)realloc(items, (maxcount + 1) * sizeof(item_t *));
		assert(_items != NULL);

		items = _items;

		items[0] = NULL;
	}

	place(item, ++count, false);
	item->heap = this;
}

void bq_heap_t::remove(item_t *item) throw() {
	assert(item->ind);
	assert(item->heap == this);

	item_t *_item = get(count--);

	if(_item != item)
		place(_item, item->ind, true);

	item->ind = 0;
	item->heap = NULL;
}

bq_heap_t::~bq_heap_t() throw() {
	assert(!count);
	if(items) free(items);
}

void bq_heap_t::validate() {
#ifdef DEBUG_BQ
	for(size_t i = count; i > 1; --i) {
		if(!hcmp(get(i / 2), get(i)))
			fatal("bq_cont_heap validation failed");
	}
#endif
}

} // namespace pd
