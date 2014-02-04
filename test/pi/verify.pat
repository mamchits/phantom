null;
ok
---------
0.5;
ok
---------
3;
ok
---------
86859585858;
ok
---------
{};
ok
---------
[];
ok
---------
[ 1 ];
ok
---------
[ "qqeqeqweqe", 0.5, 86859585858, [], [[]], ["qqeqeqweqe", 0.5, 86859585858], [ 86859585858, "qqeqeqweqe" ] ];
ok
---------
{ "jjjj": 2 };
ok
---------
{
	"00": 0, "01": 1, "02": 2, "03": 3, "04": 4,
	"05": 5, "06": 6, "07": 7, "08": 8, "09": 9
};

ok
---------
null;
---------
pi_t::verify err: wrong buffer size, lev = 0, obj = 0, req = 0, bound = 0
---------
pi_t::verify err: junk between root and root data, lev = 0, obj = 0, req = 2, bound = 3
---------
[ ];
---------
{ };
---------
"";
---------
pi_t::verify err: ivalid object type, lev = 1, obj = 1, req = 0, bound = 0
---------
"a";
---------
pi_t::verify err: ill-placed string (var 1), lev = 1, obj = 1, req = 5, bound = 4
---------
pi_t::verify err: ill-placed string (var 2), lev = 1, obj = 1, req = 5, bound = 4
---------
[ 1 ];
---------
pi_t::verify err: ill-placed array (var 1), lev = 2, obj = 3, req = 5, bound = 4
---------
[ 1, [ 1 ] ];
---------
pi_t::verify err: ill-placed array (var 2), lev = 2, obj = 4, req = 8, bound = 7
---------
pi_t::verify err: junk between array items and items data, lev = 1, obj = 1, req = 5, bound = 7
---------
[ "a", "b" ];
---------
pi_t::verify err: ill-placed array (var 2), lev = 2, obj = 4, req = 8, bound = 7
---------
pi_t::verify err: junk between array items and items data, lev = 1, obj = 1, req = 5, bound = 6
---------
{ "a": null, [ { }, _1222, { "q": "p" } ]: "12345", "c": any };
---------
pi_t::verify err: nonzero map index padding, lev = 1, obj = 1, req = 10, bound = 11
---------
{ "jjjj": 2 };
---------
pi_t::verify err: nonzero string padding, lev = 2, obj = 3, req = 8, bound = 9
---------
pi_t::verify err: nonzero string padding, lev = 2, obj = 3, req = 8, bound = 9
---------
