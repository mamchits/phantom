// This file is part of the pd::base library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#define __gcc_version(maj, min, patch) ((maj << 16) + (min << 8) + patch)
#define __gcc_version_current __gcc_version(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#define __noreturn __attribute__((noreturn))
#define __format(x) __attribute__((format(printf, x, x + 1)))
#define __hidden __attribute__((visibility ("hidden")))
#define __init_priority(x) __attribute__((init_priority(x)))
#define __noinline __attribute__((noinline))
#define __packed __attribute__((packed))
#define __weak __attribute__((weak))
#define __aligned(x) __attribute__((aligned(x)))
