#include "supp.C_"

static inline void __test(string_t const &map, string_t const &key) {
	pi_t::root_t *root_map = NULL, *root_key = NULL;
	try {
		in_t::ptr_t ptr = map;
		root_map = pi_t::parse_text(ptr, mem_test);
		ptr = key;
		root_key = pi_t::parse_text(ptr, mem_test);

		if(root_map->value.type() != pi_t::_map)
			throw exception_log_t(log::error, "test err: not a map");
		
		pi_t::map_t const &_map = root_map->value.__map();

		pi_t const &key = root_key->value;

		pi_t const *val = _map.lookup(key);
		if(!val)
			throw exception_log_t(log::error, "test err: not found");

		out.print(*val).lf();
	}
	catch(exception_t const &) { }

	if(root_map)
		mem_test.free(root_map);

	if(root_key)
		mem_test.free(root_key);
}

#define test(x,y) __test(STRING(x), STRING(y))

#define MAP1 \
	"\t\"Key1\": [1, 2, 3, 4]" \

#define MAP2 \
	"\t\"Key2\": [2, 3, 4, 1], \n" \
	"\t\"Key3\": [3, 4, 1, 2], \n" \
	"\t\"Key4\": [4, 1, 2, 3]"

#define MAP "{\n" MAP1 ",\n" MAP2 "\n};\n"



int main() {
	test(MAP, "\"Key1\";");
	test(MAP, "\"Key2\";");
	test(MAP, "\"Key3\";");
	test(MAP, "\"Key4\";");
	test(MAP, "\"abc\";");
	test("{ \"e\" : 1,  \"e\" : 2 };", "\"e\";");

	test(
		"{\n" MAP1 ",\n[ \"a\", [], {} ] : _1234,\n" MAP2 "\n};\n",
		"[ \"a\", [], {} ];"
	);
}
