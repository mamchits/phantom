// This file is part of the pd::http library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "limits_config.H"

#include <pd/base/config.H>

namespace pd { namespace http {

limits_t::limits_t(config_t const &config) :
	line(config.line), field_num(config.field_num),
	field_size(config.field), entity_size(config.entity) { }

namespace limits {
config_binding_sname(limits_t);
config_binding_value(limits_t, line);
config_binding_value(limits_t, field_num);
config_binding_value(limits_t, field);
config_binding_value(limits_t, entity);
}

}} // namespace pd::http
