// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#include <pd/base/string.H>

#pragma GCC visibility push(default)

namespace pd {

string_t string_file(str_t const &filename);
string_t string_file(char const *sys_filename_z);

} // namespace pd

#pragma GCC visibility pop
