""
pi_t::parse err: root object expected, line = 1, pos = 1, abspos = 0
---------
"\t\t123   \n"
123
---------
"null"
null
---------
"true"
true
---------
"false"
false
---------
"_12bc"
_12bc
---------
"0.0"
0.000000e+00
---------
"\"12345678\""
"12345678"
---------
"18446744073709551615"
18446744073709551615
---------
"123.2334493333333333333333333333333333333333333333333333333"
1.232334e+02
---------
"[ ]"
[ ]
---------
"{ }"
{ }
---------
"[\n\t\"12345678\", \"123\"\n]"
[ "12345678", "123" ]
---------
"[ [ [], [ ], \"12345\", \"1234\" ], \"12345\", [ \"qwert\", \"jfhtu\", [ \"ppppp\" ]]]"
[
	[ [ ], [ ], "12345", "1234" ],
	"12345",
	[ "qwert", "jfhtu", [ "ppppp" ] ]
]
---------
"{ \"abcdef\": \"123456\", \"qwerty\": [ \"\", \"a\", \"q\" ], \"asddf\" : { \"m\": \"none\", \"l\": { } }}"
{
	"abcdef": "123456",
	"qwerty": [ "", "a", "q" ],
	"asddf": { "m": "none", "l": { } }
}
---------
"[ 0, 1, -1]"
[ 0, 1, -1 ]
---------
"{ \"l\": 1000000000,  \"m\": -1000000000 }"
{ "l": 1000000000, "m": 18446744072709551616 }
---------
"{ \"true\": true, \"false\": false, \"null\": null }"
{
	"true": true,
	"false": false,
	"null": null
}
---------
"{ }"
{ }
---------
"{ \"\" : \"\" }"
{ "": "" }
---------
"[ -8240279749604792260, -12345.74645, 2.5e-10 ]"
[ 10206464324104759356, -1.234575e+04, 2.500000e-10 ]
---------
"\"\\\"\\\\\\n\\r\\b\\f\\t\""
"\"\\\n\r\b\f\t"
---------
"\"\320\272\320\260\320\273\320\276\321\210\320\260\""
"калоша"
---------
"\"\\\"\\320\\272\\320\\260\\320\\273\\320\\276\\321\\210\\320\\260\\\"\""
"\"калоша\""
---------
"\"\\u0410\\u0430\\u0411\\u0431\\u0412\\u0432\\u0413\\u0433\\u0414\\u0434\\u0415\\u0435\\u0401\\u0451\\u0416\\u0436\\u0417\\u0437\\u0438\\u0419\\u0439\\u041a\\u043a\\u041b\\u043b\\u041c\\u043c\\u041d\\u043d\\u041e\\u043e\\u041f\\u043f\\u0420\\u0440\\u0421\\u0441\\u0422\\u0442\\u0423\\u0443\\u0424\\u0444\\u0425\\u0445\\u0426\\u0446\\u0427\\u0447\\u0428\\u0448\\u0429\\u0449\\u042a\\u044a\\u042d\\u044b\\u042c\\u044c\\u042d\\u044d\\u042e\\u044e\\u042f56789\\u044f\""
"АаБбВвГгДдЕеЁёЖжЗзиЙйКкЛлМмНнОоПпР�\200СсТтУуФфХхЦцЧчШшЩщЪъЭыЬьЭэЮюЯ56789я"
---------
"\"\\\"\""
"\""
---------
"\"\\/\""
"/"
---------
""
pi_t::parse err: root object expected, line = 1, pos = 1, abspos = 0
---------
"\n"
pi_t::parse err: root object expected, line = 2, pos = 1, abspos = 1
---------
"//"
pi_t::parse err: root object expected, line = 1, pos = 1, abspos = 0
---------
"\"1234\n\""
pi_t::parse err: control character in the string, line = 1, pos = 6, abspos = 5
---------
"\"1234\t\""
pi_t::parse err: control character in the string, line = 1, pos = 6, abspos = 5
---------
"1234"
1234
---------
"1234r"
pi_t::parse err: junk behind the root object, line = 1, pos = 5, abspos = 4
---------
"[1234r]"
pi_t::parse err: comma or end of array expected, line = 1, pos = 6, abspos = 5
---------
"[1234null]"
pi_t::parse err: comma or end of array expected, line = 1, pos = 6, abspos = 5
---------
"[0.0any]"
pi_t::parse err: comma or end of array expected, line = 1, pos = 5, abspos = 4
---------
"[ 1234r"
pi_t::parse err: comma or end of array expected, line = 1, pos = 7, abspos = 6
---------
"87368473098404839489287346924"
pi_t::parse err: integer overflow, line = 1, pos = 1, abspos = 0
---------
"-9223372036854775808"
9223372036854775808
---------
"-9223372036854775809"
pi_t::parse err: integer overflow, line = 1, pos = 1, abspos = 0
---------
"123.23344e15552"
pi_t::parse err: floating point number parsing error, line = 1, pos = 1, abspos = 0
---------
"1234e"
pi_t::parse err: floating point number parsing error, line = 1, pos = 1, abspos = 0
---------
"medved"
pi_t::parse err: unknown enum value, line = 1, pos = 1, abspos = 0
---------
".0"
pi_t::parse err: root object expected, line = 1, pos = 1, abspos = 0
---------
"{\n\t\"qwe\": }"
pi_t::parse err: object expected, line = 2, pos = 9, abspos = 10
---------
"{\n\"abc\": 1, q"
pi_t::parse err: unknown enum value, line = 2, pos = 11, abspos = 12
---------
"\"\\r\\t\\y\\v\\g\\y\""
pi_t::parse err: illegal escape character, line = 1, pos = 7, abspos = 6
---------
"\""
pi_t::parse err: unexpected end of string, line = 1, pos = 2, abspos = 1
---------
"\"abc"
pi_t::parse err: unexpected end of string, line = 1, pos = 5, abspos = 4
---------
"\"a\\"
pi_t::parse err: unexpected end of string, line = 1, pos = 4, abspos = 3
---------
"\"a\\1"
pi_t::parse err: unexpected end of string, line = 1, pos = 5, abspos = 4
---------
"\"a\\17"
pi_t::parse err: unexpected end of string, line = 1, pos = 6, abspos = 5
---------
"\"a\\1a"
pi_t::parse err: not an octal digit, line = 1, pos = 5, abspos = 4
---------
"\"ab[cd]ef\""
"ab[cd]ef"
---------
