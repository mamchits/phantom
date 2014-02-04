// This file is part of the pd::base library.
// Copyright (C) 2009-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2009-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#pragma once

#pragma GCC visibility push(default)

namespace pd { namespace vtempl {

template<int ...>
struct ints_t { };

template<int i, typename ints_t>
struct __idxs_t;

template<int ...res>
struct __idxs_t<0, ints_t<res...>> {
	typedef ints_t<res...> type_t;
};

template<int i, int ...res>
struct __idxs_t<i, ints_t<res...>> {
	typedef typename __idxs_t<i - 1, ints_t<i - 1, res...>>::type_t type_t;
};

template<int i>
struct idxs_t : __idxs_t<i, ints_t<>> { };

// -----------------------------------

template<int i, typename ...args_t>
struct ind_t;

template<int i>
struct ind_t<i> {
	typedef void type_t;
};

template<typename x_t, typename ...others_t>
struct ind_t<0, x_t, others_t...> {
	typedef x_t type_t;
};

template<int i, typename x_t, typename ...others_t>
struct ind_t<i, x_t, others_t...> {
	typedef typename ind_t<i - 1, others_t...>::type_t type_t;
};

}} // namespace pd::vtempl

#pragma GCC visibility pop
