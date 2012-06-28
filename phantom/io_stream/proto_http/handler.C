// This file is part of the phantom::io_stream::proto_http module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "handler.H"

namespace phantom { namespace io_stream { namespace proto_http {

namespace handler {
config_binding_sname(handler_t);
config_binding_type(handler_t, verify_t);
config_binding_value(handler_t, verify);
config_binding_value(handler_t, scheduler);
config_binding_value(handler_t, switch_prio);
}

}}} // namespace phantom::io_stream::proto_http
