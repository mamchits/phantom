dst:
"123456"
src:
"abcdef"
path:
[ ]
res:
"abcdef"
---------
dst:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
src:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
path:
[ "Key3", 1 ]
res:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [
		3,
		[
			[ "0:abc", "0:def", "0:ghi" ],
			[ "1:abc", "1:def", "1:ghi" ],
			[ "2:abc", "2:def", "2:ghi" ],
			[ "3:abc", "3:def", "3:ghi" ]
		],
		1,
		2
	],
	"Key4": [ 4, 1, 2, 3 ]
}
---------
dst:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
src:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
path:
[ "Key3" ]
res:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [
		[ "0:abc", "0:def", "0:ghi" ],
		[ "1:abc", "1:def", "1:ghi" ],
		[ "2:abc", "2:def", "2:ghi" ],
		[ "3:abc", "3:def", "3:ghi" ]
	],
	"Key4": [ 4, 1, 2, 3 ]
}
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
path:
[ 0, 1 ]
res:
[
	[
		"0:abc",
		{
			"Key1": [ 1, 2, 3, 4 ],
			"Key2": [ 2, 3, 4, 1 ],
			"Key3": [ 3, 4, 1, 2 ],
			"Key4": [ 4, 1, 2, 3 ]
		},
		"0:ghi"
	],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
path:
[ 0 ]
res:
[
	{
		"Key1": [ 1, 2, 3, 4 ],
		"Key2": [ 2, 3, 4, 1 ],
		"Key3": [ 3, 4, 1, 2 ],
		"Key4": [ 4, 1, 2, 3 ]
	},
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
---------
dst:
[
	null,
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
path:
[ 0 ]
res:
[
	{
		"Key1": [ 1, 2, 3, 4 ],
		"Key2": [ 2, 3, 4, 1 ],
		"Key3": [ 3, 4, 1, 2 ],
		"Key4": [ 4, 1, 2, 3 ]
	},
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ],
	true
]
src:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
path:
[ 4 ]
res:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ],
	{
		"Key1": [ 1, 2, 3, 4 ],
		"Key2": [ 2, 3, 4, 1 ],
		"Key3": [ 3, 4, 1, 2 ],
		"Key4": [ 4, 1, 2, 3 ]
	}
]
---------
dst:
{
	"Key0": 1,
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
src:
[ 5.000000e-01, 7.000000e-01 ]
path:
[ "Key0" ]
res:
{
	"Key0": [ 5.000000e-01, 7.000000e-01 ],
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 2 ]
res:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	null,
	[ "3:abc", "3:def", "3:ghi" ]
]
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, 0, 0 ]
pi_t::path err: destination object is not a map or array, lev = 2
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, 5 ]
pi_t::path err: index too big, lev = 1
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, -3 ]
pi_t::path err: negative index, lev = 1
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ "Key3" ]
pi_t::path err: only small integer may index an array, lev = 0
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, [ 0 ] ]
pi_t::path err: only small integer may index an array, lev = 1
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, { } ]
pi_t::path err: only small integer may index an array, lev = 1
---------
dst:
[
	[ "0:abc", "0:def", "0:ghi" ],
	[ "1:abc", "1:def", "1:ghi" ],
	[ "2:abc", "2:def", "2:ghi" ],
	[ "3:abc", "3:def", "3:ghi" ]
]
src:
null
path:
[ 0, null ]
pi_t::path err: only small integer may index an array, lev = 1
---------
dst:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
src:
null
path:
[ 0, 0 ]
pi_t::path err: no such object in destination map, lev = 0
---------
dst:
{
	"Key1": [ 1, 2, 3, 4 ],
	"Key2": [ 2, 3, 4, 1 ],
	"Key3": [ 3, 4, 1, 2 ],
	"Key4": [ 4, 1, 2, 3 ]
}
src:
null
path:
[ "abc", 0 ]
pi_t::path err: no such object in destination map, lev = 0
---------
dst:
[
	12345,
	[
		1,
		{
			4: 5,
			{
				"a": null,
				[ { }, _1222, { "q": "p" } ]: "12345",
				"c": any
			}: null,
			1: 2
		}
	],
	8
]
src:
[ 1, 2, 3, 4 ]
path:
[ 1, 1, { "a": null, [ { }, _1222, { "q": "p" } ]: "12345", "c": any } ]
res:
[
	12345,
	[
		1,
		{
			4: 5,
			{
				"a": null,
				[ { }, _1222, { "q": "p" } ]: "12345",
				"c": any
			}: [ 1, 2, 3, 4 ],
			1: 2
		}
	],
	8
]
---------
