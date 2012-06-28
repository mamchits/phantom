#include <pd/base/ipv6.H>
#include <pd/base/out_fd.H>
#include <pd/base/config_helper.H>
#include <pd/base/exception.H>

using namespace pd;

static char outbuf[1024];
static out_fd_t out(outbuf, sizeof(outbuf), 1);

static void __noreturn error(in_t::ptr_t const &, char const *msg) {
	throw msg;
}

static address_ipv6_t parse_adress(string_t const &str) {
	in_t::ptr_t p(str);
	address_ipv6_t addr;
	p.parse(addr, error);
	return addr;
}

static void parse_address_test(string_t const &str) {
	try {
		address_ipv6_t addr = parse_adress(str);

		out
			('\"')(str)('\"')('-')('>')('\"').print(addr)
			('\"')(' ')('\"').print(addr, "r")('\"').lf()
		;
	}
	catch(char const *estr) {
		out(CSTR("*** "))('\"')(str)('\"')(' ')(str_t(estr, strlen(estr))).lf();
	}
	catch(exception_t const &ex) {
		out(CSTR("--- "))('\"')(str)('\"')(' ')(ex.msg()).lf();
	}
}

static network_ipv6_t parse_network(string_t const &str) {
	in_t::ptr_t p(str);
	network_ipv6_t net;
	p.parse(net, error);
	return net;
}

static void parse_network_test(string_t const &str) {
	try {
		network_ipv6_t net = parse_network(str);

		out
			('\"')(str)('\"')('-')('>')('\"').print(net)
			('\"')(' ')('\"').print(net, "r")('\"').lf()
		;
	}
	catch(char const *estr) {
		out(CSTR("*** "))('\"')(str)('\"')(' ')(str_t(estr, strlen(estr))).lf();
	}
	catch(exception_t const &ex) {
		out(CSTR("--- "))('\"')(str)('\"')(' ')(ex.msg()).lf();
	}
}

static string_t strs_addr[] = {
	string_t::empty,
	STRING(":"),
	STRING(":1"),
	STRING("::"),
	STRING("::1"),
	STRING("1:2:3:4:"),
	STRING("1:2:3:4:q"),
	STRING("1:2:3:4"),
	STRING("1::2::3"),
	STRING("1:2:3:4:5:6:7:8"),
	STRING(":1:2:3:4:5:6:7:8"),
	STRING("1:2:3:4:5:6:7:8:9"),
	STRING("1:"),
	STRING("1::"),
	STRING("1::123456"),
	STRING("12345::"),
	STRING("fe80::217:42ff:fe0e:cc0b"),
	STRING("193.0.0.193"),
	STRING("193.0.0"),
	STRING("::ffff:c100:c1"),
};

static string_t strs_net[] = {
	string_t::empty,
	STRING("::/0"),
	STRING("192.168.0.0/24"),
	STRING("::ffff:c0a8:0/120"),
	STRING("fe80::217:42ff:fe0e:0/112"),
	STRING("fe80::217:42ff:fe0e:0/100"),
};

extern "C" int main() {
	for(
		string_t *p = strs_addr;
		p < strs_addr + sizeof(strs_addr)/sizeof(strs_addr[0]); ++p
	) {
		 parse_address_test(*p);
	}

	for(
		string_t *p = strs_net;
		p < strs_net + sizeof(strs_net)/sizeof(strs_net[0]); ++p
	) {
		 parse_network_test(*p);
	}
}
