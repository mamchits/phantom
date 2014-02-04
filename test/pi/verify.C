#include "supp.C_"

static inline void __test(string_t const &string) {
	pi_t::root_t *root = NULL;

	out.print(string).lf();
	try {
		in_t::ptr_t ptr = string;
		root = pi_t::parse_text(ptr, mem_test);
		pi_t::verify((char const *)root, root->size * sizeof(pi_t));
		out('o')('k').lf();
	}
	catch(exception_t const &) {
		if(root) {
			uint32_t const *p = (uint32_t const *)root;
			for(pi_t::_size_t size = root->size; size--;)
				out(' ').print(*(p++), "08x");

			out.lf();
		}
	}

	if(root)
		mem_test.free(root);

	out(CSTR("---------")).lf().flush_all();
}

#define test(x) __test(STRING(x))

static inline void __test_array(uint32_t const *arr, size_t size) {
	try {
		pi_t::root_t const *_root = pi_t::verify((char const *)arr, size * sizeof(pi_t));
		out.print(_root->value)(';').lf();
	}
	catch(exception_t const &) { }

	out(CSTR("---------")).lf().flush_all();
}

#define test_arr(arr...) \
	do { \
		static uint32_t array[] = { arr }; \
		__test_array(array, sizeof(array) / sizeof(array[0])); \
	} while(0)

int main() {
	test("null;");
	test("0.5;");
	test("3;");
	test("86859585858;");
	test("{};");
	test("[];");
	test("[ 1 ];");
	test(
		"[ \"qqeqeqweqe\", 0.5, 86859585858, [], [[]], [" 
				"\"qqeqeqweqe\", 0.5, 86859585858"
		"], [ 86859585858, \"qqeqeqweqe\" ] ];"
	);
	test("{ \"jjjj\": 2 };");
	test(
		"{\n"
		"\t\"00\": 0, \"01\": 1, \"02\": 2, \"03\": 3, \"04\": 4,\n"
		"\t\"05\": 5, \"06\": 6, \"07\": 7, \"08\": 8, \"09\": 9\n"
		"};\n"
	);

	test_arr(0x00000002, 0x00000000);  // null
	test_arr(0x00000002, 0x00000000, 0x00000000); // wrong buffer size
	test_arr(0x00000003, 0x00000000, 0x00000000); 
	test_arr(0x00000002, 0xc0000000);  // []
	test_arr(0x00000002, 0xe0000000);  // {}
	test_arr(0x00000002, 0x80000000);  // ""
	test_arr(0x00000002, 0xa0000000);  // ivalid object type

	test_arr(0x00000004, 0x80000001, 0x00000001, 0x00000061); // "a"
	test_arr(0x00000004, 0x80000002, 0x00000004, 0x00000061); //
	test_arr(0x00000004, 0x80000001, 0x00000004, 0x00000061); //

	test_arr(0x00000004, 0xc0000001, 0x00000001, 0x20000001); // [ 1 ]
	test_arr(0x00000004, 0xc0000001, 0x00000001, 0xc0000001);

	test_arr(
		0x00000007, 0xc0000001, 0x00000002, 0x20000001, 0xc0000001, 0x00000001,
		0x20000001); // [ 1 [ 1 ] ]

	test_arr(
		0x00000007, 0xc0000001, 0x00000002, 0x00000001, 0xc0000002, 0x00000001,
		0x00000001);

	test_arr(
		0x00000007, 0xc0000001, 0x00000002, 0x00000001, 0xc0000000, 0x00000001,
		0x00000001);

	test_arr(
		0x00000009, 0xc0000001, 0x00000002, 0x80000004, 0x80000001, 0x00000001,
		0x00000062, 0x00000001, 0x00000061); // [ "a", "b" ]

	test_arr(
		0x00000009, 0xc0000001, 0x00000002, 0x80000004, 0xc0000001, 0x00000002,
		0x00000062, 0x00000001, 0x00000061); //

	test_arr(
		0x0000000a, 0xc0000001, 0x00000002, 0x80000005, 0x80000002, 0x00000000, 0x00000001,
		0x00000062, 0x00000001, 0x00000061); //

	test_arr(
		0x0000001e, 0xe0000001, 0x00000003, 0x80000019, 0x00000000, 0xc000000b,
		0x80000007, 0x80000004, 0x00000001, 0x00010302, 0x00000000, 0x00000001,
		0x00000063, 0x00000005, 0x34333231, 0x00000035, 0x00000003, 0xe0000000,
		0x1fff1222, 0xe0000001, 0x00000001, 0x80000005, 0x80000002, 0x00000001,
		0x00000001, 0x00000070, 0x00000001, 0x00000071, 0x00000001, 0x00000061,
	);

	test_arr(
		0x0000001e, 0xe0000001, 0x00000003, 0x80000019, 0x00000000, 0xc000000b,
		0x80000007, 0x80000004, 0x00000001, 0x00010302, 0x08060000, 0x00000001,
		0x00000063, 0x00000005, 0x34333231, 0x00000035, 0x00000003, 0xe0000000,
		0x1fff1222, 0xe0000001, 0x00000001, 0x80000005, 0x80000002, 0x00000001,
		0x00000001, 0x00000070, 0x00000001, 0x00000071, 0x00000001, 0x00000061,
	);

	test_arr(
		0x00000009, 0xe0000001, 0x00000001, 0x80000003, 0x20000002, 0x00000001,
		0x00000004, 0x6a6a6a6a, 0x00000000);

	test_arr(
		0x00000009, 0xe0000001, 0x00000001, 0x80000003, 0x20000002, 0x00000001,
		0x00000004, 0x6a6a6a6a, 0x00000100);

	test_arr(
		0x00000009, 0xe0000001, 0x00000001, 0x80000003, 0x20000002, 0x00000001,
		0x00000004, 0x6a6a6a6a, 0x01000000);
}
