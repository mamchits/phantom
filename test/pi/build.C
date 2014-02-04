#include "supp.C_"

#include <pd/pi/pi_pro.H>

static inline pi_t::root_t *__create(string_t const &string) {
	try {
		in_t::ptr_t ptr = string;
		pi_t::root_t *root = pi_t::parse_text(ptr, mem_test);
		// out.print(root->value, "10").lf();
		return root;
	}
	catch(exception_t const &) { }

	return NULL;
}

static inline void test(pi_t::pro_t const &pro) {
	pi_t::root_t *root = NULL;

	try {
		root = pi_t::build(pro, mem_test);
		pi_t::verify((char const *)root, root->size * sizeof(pi_t));
		out.print(root->value, "10").lf();
	}
	catch(exception_t const &ex) {
		uint32_t const *p = (uint32_t const *)root;
		for(pi_t::_size_t size = root->size; size--;)
			out(' ').print(*(p++), "08x");

		out.lf();
	}

	if(root)
		mem_test.free(root);

	out(CSTR("---------------------")).lf();
	out.flush_all();
}


#define create(x) __create(STRING(x))

typedef pi_t::pro_t pro_t;


struct mem_buf_t : public pd::pi_t::mem_t {
	char *buf;
	size_t size;

	virtual void *alloc(size_t _size) const {
		if(_size > size)
			abort();

		return buf;
	}

	virtual void free(void *) const { }

	inline mem_buf_t(char *_buf, size_t _size) throw() :
		buf(_buf), size(_size) { }

	inline ~mem_buf_t() throw() { }
};

static pd::pi_t::root_t *test_pi_string(str_t str) {
	size_t name_size = str.size();
	char const *name = str.ptr();

	size_t b_size1 = pd::pi_t::root_t::_size();
	size_t b_size2 = pd::pi_t::string_t::_size(name_size);

	size_t buf_size = (b_size1 + b_size2) * sizeof(pd::pi_t);
	char buf[buf_size];

	pd::str_t name_str(name, name_size);
	pd::pi_t::pro_t name_pro(name_str);

	pd::pi_t::root_t *root = pd::pi_t::build(name_pro, mem_buf_t(buf, sizeof(buf)));
	return root;
}

static void test_aux() {
	test_pi_string(CSTR(""));
	test_pi_string(CSTR("0"));
	test_pi_string(CSTR("01"));
	test_pi_string(CSTR("012"));
	test_pi_string(CSTR("0123"));
}

int main() {
	pi_t::root_t *root = create("[ 1, 2, null, { \"abc\" : 3, 3 : \"abc\" }];");

	if(!root)
		return 1;
	{
		pro_t pro(root->value);
		test(pro);
	}
	{
		pro_t const pros[5] = { root->value, root->value, root->value, root->value, root->value };
		pro_t::array_t array = { 5, pros };
		pro_t pro(array);
		test(pro);
	}
	{
		pro_t::map_t::item_t items[5];
		items[3].value = root->value;
		pro_t::map_t map = { 5, items };
		pro_t pro(map);
		test(pro);
	}
	{
		str_t str(CSTR("abcd"));
		pro_t::map_t::item_t items[2];

		items[0].key = pro_t::int_t(-1); items[0].value = root->value;
		items[1].key = pro_t::uint_t(~0ULL); items[1].value = str;

		pro_t::map_t map = { 2, items };
		pro_t pro(map);

		test(pro);
	}
	{
		str_t str(CSTR("lkjhg"));
		pro_t::item_t
			item1(str, NULL),
			item2(pro_t::uint_t(1234), &item1),
			item3(root->value, &item2)
		;
		
		pro_t pro(&item3);
		test(pro);
	}
	{
		pro_t::array_t arr(0, NULL);
		pro_t::pair_t
			pair1(pro_t::enum_t(pi_t::_any), arr, NULL),
			pair2(pro_t::enum_t(pi_t::_true), root->value, &pair1),
			pair3(
				pro_t::null_array,
				pro_t::null_map,
				&pair2
			);
		;
		pro_t pro(&pair3);
		test(pro);
		pro_t::pair_t pair4(&pair3, &pair3, &pair3);
		test(pro_t(&pair4));
	}

	{
		str_t str = CSTR("qq");
		auto array = pro_t::array(1, 2, 0.5, str);
		test(pro_t(array));
		auto array2 = pro_t::array(array, array);
		test(pro_t(array2));
	}

	{
		str_t s1 = CSTR("__1"), s2 = CSTR("__2"), s3 = CSTR("__3"), s4 = CSTR("__4");
		auto map = pro_t::map(
			pro_t::pair(1, s1),
			pro_t::pair(2u, s2),
			pro_t::pair(3l, s3),
			pro_t::pair(4ul, s4)
		);

		test(pro_t(map));

		auto map1 = pro_t::map(
			pro_t::pair(1, map),
			pro_t::pair(2, map)
		);

		test(map1);
	}

	mem_test.free(root);

	test_aux();
}
