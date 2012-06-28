// This file is part of the phantom::io_stream module.
// Copyright (C) 2006-2012, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2012, YANDEX LLC.
// This module may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "acl.H"

#include <pd/base/config_enum.H>

namespace phantom { namespace io_stream {

namespace acl {
config_enum_internal_sname(acl_t, policy_t);
config_enum_internal_value(acl_t, policy_t, allow);
config_enum_internal_value(acl_t, policy_t, deny);
}

}} // namespace phantom::io_stream
