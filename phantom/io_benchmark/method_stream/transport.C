// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "transport.H"

#include <unistd.h>

namespace phantom { namespace io_benchmark { namespace method_stream {

conn_t::~conn_t() throw() { ::close(fd); }

}}} // namespace phantom::io_benchmark::method_stream
