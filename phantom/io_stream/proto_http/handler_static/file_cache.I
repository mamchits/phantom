// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include "path_translation.H"

#include <pd/base/list.H>
#include <pd/base/list2.H>
#include <pd/base/ref.H>
#include <pd/base/time.H>
#include <pd/base/mutex.H>

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

struct file_t : public ref_count_atomic_t {
	string_t sys_name_z;
	timeval_t access_time, check_time;
	int fd;
	dev_t dev;
	ino_t ino;
	timeval_t mtime;
	off_t size;
	string_t mtime_string;

	inline operator bool() const throw() { return fd >= 0; }

	file_t(string_t const &_sys_name_z, timeval_t curtime = timeval::current()) throw();
	~file_t() throw();

	friend class ref_t<file_t>;
};

class file_cache_t {
	class age_list_t;

	class age_list_item_t : public list2_item_t<age_list_item_t> {
	protected:
		inline age_list_item_t() : list2_item_t<age_list_item_t>(this) { }

	public:
		virtual ~age_list_item_t() throw() { }

		friend class age_list_t;
	};

	class age_list_t : age_list_item_t {
	public:
		inline age_list_t() : age_list_item_t() { }

		inline ~age_list_t() throw() {
			while(prev != this)
				delete prev;
		}

		inline void touch(age_list_item_t *item) {
			item->next->prev = item->prev;
			item->prev->next = item->next;

			(item->next = next)->prev = item;
			(item->prev = this)->next = item;
		}

		inline void expire() {
			assert(prev != this);

			delete prev;
		}
	};

	struct node_t : public list_item_t<node_t>, age_list_item_t {
		string_t key;
		ref_t<file_t> file;

		inline node_t(
			node_t *&list, string_t const &_key, string_t const &sys_name_z
		) :
			list_item_t<node_t>(this, list), age_list_item_t(),
			key(_key.copy()), file(new file_t(sys_name_z)) { }

		inline ~node_t() throw() { }

		friend class file_cache_t;
	};

	struct bucket_t {
		node_t *list;

		inline bucket_t() throw() : list(NULL) { }
		inline ~bucket_t() throw() { while(list) delete list; }
	};

	size_t cache_size;
	size_t count;
	string_t root;
	path_translation_t const &translation;
	interval_t check_time;
	mutex_t mutex;

	bucket_t *buckets;
	age_list_t age_list;

public:
	inline file_cache_t(
		size_t _cache_size, string_t const &_root,
		path_translation_t const &_translation, interval_t _check_time
	) :
		cache_size(_cache_size), count(0),
		root(_root), translation(_translation), check_time(_check_time) {

		buckets = cache_size ? new bucket_t[cache_size] : NULL;
	}

	inline ~file_cache_t() throw() {
		if(cache_size)
			delete [] buckets;
	}

	ref_t<file_t> find(string_t const &path);
};

}}}} // namespace phantom::io_stream::proto_http::handler_static
