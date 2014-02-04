// This file is part of the phantom::io_stream::proto_http::handler_static module.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "file_cache.I"

#include <pd/http/http.H>

#include <pd/base/exception.H>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

namespace phantom { namespace io_stream { namespace proto_http { namespace handler_static {

file_t::file_t(string_t const &_sys_name_z, timeval_t curtime) throw() :
	sys_name_z(_sys_name_z) {

	struct stat st;

	fd = ::open(sys_name_z.ptr(), O_RDONLY, 0);
	if(fd < 0) {
		switch(errno) {
			case EMFILE: case ENFILE: case ENOMEM:
				log_error("open: %m");
				fd = -2;
		}
	}

	if(fd >= 0) {
		if(::fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)) {
			::close(fd);
			fd = -1;
		}
	}

	if(fd >= 0) {
		dev = st.st_dev;
		ino = st.st_ino;
		size = st.st_size;
		mtime_string = http::time_string(
			mtime = timeval::unix_origin + st.st_mtime * interval::second
		);
	}

	access_time = check_time = curtime;
}

file_t::~file_t() throw() { if(fd >= 0) ::close(fd); }

void check(ref_t<file_t> &file_ref, interval_t const &check_time) {
	file_t *file = file_ref;
	timeval_t time = timeval::current();

	if(time > file->check_time + check_time) {
		struct stat st;
		bool stat_res = (::stat(file->sys_name_z.ptr(), &st) >= 0 && S_ISREG(st.st_mode));
		if(
			stat_res
				? !*file || file->dev != st.st_dev || file->ino != st.st_ino
				: *file
		) {
			file_ref = file = new file_t(file->sys_name_z, time);
		}
		else if(stat_res) {
			timeval_t mtime = timeval::unix_origin + st.st_mtime * interval::second;

			if(st.st_size != file->size) {
				log_error("File \"%s\" incorrectly changed", file->sys_name_z.ptr());
				file->size = st.st_size;
			}

			if(mtime != file->mtime)
				file->mtime = mtime;
		}

		file->check_time = time;
	}

	file->access_time = time;
}

ref_t<file_t> file_cache_t::find(string_t const &path) {
	if(!cache_size)
		return ref_t<file_t>(new file_t(translation.translate(root, path)));

	bucket_t &bucket = buckets[path.fnv<ident_t>() % cache_size];

	{
		mutex_guard_t guard(mutex);

		node_t *node = bucket.list;

		for(; node; node = node->list_item_t<node_t>::next) {
			if(string_t::cmp_eq<ident_t>(path, node->key)) {
				check(node->file, check_time);
				break;
			}
		}

		if(!node) {
			new node_t(bucket.list, path, translation.translate(root, path));
			++count;
			node = bucket.list;
		}

		if(node->file->fd == -2) {
			delete node;
			--count;
			throw http::exception_t(http::code_503, "No resources to open file");
		}

		age_list.touch(node);

		ref_t<file_t> result = node->file;

		while(count > cache_size) {
			age_list.expire();
			--count;
		}

		return result;
	}
}

}}}} // namespace phantom::io_stream::proto_http::handler_static
