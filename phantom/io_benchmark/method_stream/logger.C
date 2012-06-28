// This file is part of the phantom::io_benchmark::method_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "logger.H"

#include <pd/base/config.H>

namespace phantom { namespace io_benchmark { namespace method_stream {

namespace logger {
config_binding_sname(logger_t);
config_binding_value(logger_t, level);
}

config_enum_internal_sname(logger_t, level_t);
config_enum_internal_value(logger_t, level_t, all);
config_enum_internal_value(logger_t, level_t, proto_warning);
config_enum_internal_value(logger_t, level_t, proto_error);
config_enum_internal_value(logger_t, level_t, transport_error);
config_enum_internal_value(logger_t, level_t, network_error);

}}} // namespace phantom::io_benchmark::method_stream
