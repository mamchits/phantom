#include "supp.C_"

static inline void __test(string_t const &string) {
	out.print(string, "e").lf();

	pi_t::root_t *root = NULL;

	try {
		in_t::ptr_t ptr = string;
		root = pi_t::parse_text(ptr, mem_test);
		out.print(root->value, "10").lf();
	}
	catch(exception_t const &) { }

	if(root)
		mem_test.free(root);

	out(CSTR("---------")).lf().flush_all();
}

#define test(x) __test(STRING(x))

#if 0
void test_int29(int64_t i) {
	char buf[128];
#ifdef _LP64
	fprintf(stdout, "/* %ld, %lu, %08lx */\n", i, i, i);
	snprintf(buf, sizeof(buf), "%ld", i);
#else
	fprintf(stdout, "/* %lld, %llu, %08llx */\n", i, i, i);
	snprintf(buf, sizeof(buf), "%lld", i);
#endif
	test(buf);
}
#endif

int main() {
	test("");
	test("\t\t123   \n");
	test("null");
	test("true");
	test("false");
	test("_12bc");
	test("0.0");
	test("\"12345678\"");
	test("18446744073709551615");
	test("123.2334493333333333333333333333333333333333333333333333333");

#if 0
	test_int29(pi_int29_max);
	test_int29(pi_int29_max + 1);
	test_int29(pi_int29_min);
	test_int29(pi_int29_min - 1);
#endif
	
	test("[ ]");
	test("{ }");
	test("[\n\t\"12345678\", \"123\"\n]");
	test("[ [ [], [ ], \"12345\", \"1234\" ], \"12345\", [ \"qwert\", \"jfhtu\", [ \"ppppp\" ]]]");
	test("{ \"abcdef\": \"123456\", \"qwerty\": [ \"\", \"a\", \"q\" ], \"asddf\" : { \"m\": \"none\", \"l\": { } }}");
	test("[ 0, 1, -1]");
	test("{ \"l\": 1000000000,  \"m\": -1000000000 }");
	test("{ \"true\": true, \"false\": false, \"null\": null }");
	test("{ }");
	test("{ \"\" : \"\" }");
	test("[ -8240279749604792260, -12345.74645, 2.5e-10 ]");
	test("\"\\\"\\\\\\n\\r\\b\\f\\t\"");
	test("\"\320\272\320\260\320\273\320\276\321\210\320\260\"");
	test("\"\\\"\\320\\272\\320\\260\\320\\273\\320\\276\\321\\210\\320\\260\\\"\"");
	test("\"\\u0410\\u0430\\u0411\\u0431\\u0412\\u0432\\u0413\\u0433\\u0414\\u0434\\u0415\\u0435\\u0401\\u0451\\u0416\\u0436\\u0417\\u0437\\u0438\\u0419\\u0439\\u041a\\u043a\\u041b\\u043b\\u041c\\u043c\\u041d\\u043d\\u041e\\u043e\\u041f\\u043f\\u0420\\u0440\\u0421\\u0441\\u0422\\u0442\\u0423\\u0443\\u0424\\u0444\\u0425\\u0445\\u0426\\u0446\\u0427\\u0447\\u0428\\u0448\\u0429\\u0449\\u042a\\u044a\\u042d\\u044b\\u042c\\u044c\\u042d\\u044d\\u042e\\u044e\\u042f56789\\u044f\"");
	test("\"\\\"\"");
	test("\"\\/\"");

	test("");
	test("\n");
	test("//");
	test("\"1234\n\"");
	test("\"1234\t\"");
	test("1234");
	test("1234r");
	test("[1234r]");
	test("[1234null]");
	test("[0.0any]");
	test("[ 1234r");
	test("87368473098404839489287346924");
	test("-9223372036854775808");
	test("-9223372036854775809");
	test("123.23344e15552");
	test("1234e");
	test("medved");
	test(".0");
	test("{\n\t\"qwe\": }");
	test("{\n\"abc\": 1, q");
	test("\"\\r\\t\\y\\v\\g\\y\"");
	test("\"");
	test("\"abc");
	test("\"a\\");
	test("\"a\\1");
	test("\"a\\17");
	test("\"a\\1a");
	test("\"ab\u005bcd\u005Def\"");

	return 0;
}
