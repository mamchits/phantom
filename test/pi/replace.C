#include "supp.C_"

static inline void __test(
	string_t const &dst, string_t const &src, string_t const &path
) {
	pi_t::root_t *root_dst = NULL, *root_src = NULL,
		*root_path = NULL, *root_res = NULL;

	try {
		try {
			in_t::ptr_t ptr = dst;
			root_dst = pi_t::parse_text(ptr, mem_test);

			out(CSTR("dst:")).lf();
			out.print(root_dst->value, "10").lf();
		}
		catch(...) {
			out(dst).lf();
			throw;
		}

		try {
			in_t::ptr_t ptr = src;
			root_src = pi_t::parse_text(ptr, mem_test);

			out(CSTR("src:")).lf();
			out.print(root_src->value, "10").lf();
		}
		catch(...) {
			out(src).lf();
			throw;
		}

		try {
			in_t::ptr_t ptr = path;
			root_path = pi_t::parse_text(ptr, mem_test);

			out(CSTR("path:")).lf();
			out.print(root_path->value).lf();
		}
		catch(...) {
			out(path).lf();
			throw;
		}

		if(root_path->value.type() != pi_t::_array)
			throw exception_log_t(log::error, "test err: not a path");
		
		pi_t::array_t const &_path = root_path->value.__array();

		pi_t const &_src = root_src->value;

		root_res = pi_t::replace(*root_dst, _path, _src, mem_test);

		out(CSTR("res:")).lf();
		out.print(root_res->value, "10").lf();
	}
	catch(exception_t const &) { }

	if(root_dst)
		mem_test.free(root_dst);

	if(root_path)
		mem_test.free(root_path);

	if(root_src)
		mem_test.free(root_src);

	if(root_res)
		mem_test.free(root_res);

	out(CSTR("---------")).lf().flush_all();
}

#define test(x,y,z) __test(STRING(x), STRING(y),  STRING(z))

#define ARRAY1 \
	"\t[ \"0:abc\", \"0:def\", \"0:ghi\" ],\n" \
	"\t[ \"1:abc\", \"1:def\", \"1:ghi\" ],\n" \
	"\t[ \"2:abc\", \"2:def\", \"2:ghi\" ]" 

#define ARRAY2 \
	"\t[ \"3:abc\", \"3:def\", \"3:ghi\" ]\n" 

#define ARRAY "[\n" ARRAY1 ",\n" ARRAY2 "\n];\n"

#define MAP1 \
	"\t\"Key1\": [1, 2, 3, 4]" \

#define MAP2 \
	"\t\"Key2\": [2, 3, 4, 1],\n" \
	"\t\"Key3\": [3, 4, 1, 2],\n" \
	"\t\"Key4\": [4, 1, 2, 3]"

#define MAP "{\n" MAP1 ",\n" MAP2 "\n};\n"

int main() {
	test("\"123456\"", "\"abcdef\"", "[]");
	test(MAP, ARRAY, "[ \"Key3\", 1]");
	test(MAP, ARRAY, "[ \"Key3\"]");
	test(ARRAY, MAP, "[ 0, 1 ]");
	test(ARRAY, MAP, "[ 0 ]");
	test("[\n\tnull,\n" ARRAY1 ",\n" ARRAY2 "];\n", MAP, "[ 0 ]");
	test("[\n" ARRAY1 ",\n" ARRAY2 ",\n\ttrue\n];\n", MAP, "[ 4 ]");
	test(
		"{\n\t\"Key0\": 1,\n" MAP1 ",\n" MAP2 "\n};\n",
		"[ 0.5, 0.7 ]",
		"[ \"Key0\" ]"
	);
	test(ARRAY, "null", "[ 2 ]");

	test(ARRAY, "null", "[ 0, 0, 0 ]");
	test(ARRAY, "null", "[ 0, 5 ]");
	test(ARRAY, "null", "[ 0, -3 ]");
	test(ARRAY, "null", "[ \"Key3\" ]");
	test(ARRAY, "null", "[ 0, [ 0 ] ]");
	test(ARRAY, "null", "[ 0, { }  ]");
	test(ARRAY, "null", "[ 0, null ]");
	test(MAP, "null", "[ 0, 0 ]");
	test(MAP, "null", "[ \"abc\", 0]");

#define KEY "{ \"a\" : null, [{}, _1222, { \"q\" : \"p\" }] : \"12345\", \"c\" : any }"

	test(
		"[ 12345, [ 1, { 4 : 5, " KEY " : null, 1:2 } ], 8 ];",
		"[ 1, 2, 3, 4 ];",
		"[ 1, 1, " KEY "];"
//		"[ 1, 1, 4 ];"
	);

	return 0;
}

