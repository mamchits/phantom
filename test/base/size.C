#include <pd/base/size.H>
#include <pd/base/out_fd.H>

using namespace pd;

static char obuf[1024];
static out_fd_t out(obuf, sizeof(obuf), 1);

static void test_prec(sizeval_t s) {
	out.print(14, "02")(':').print(s, ".14").lf();
	out.print(13, "02")(':').print(s, ".13").lf();
	out.print(12, "02")(':').print(s, ".12").lf();
	out.print(11, "02")(':').print(s, ".11").lf();
	out.print(10, "02")(':').print(s, ".10").lf();
	out.print(9, "02")(':').print(s, ".9").lf();
	out.print(8, "02")(':').print(s, ".8").lf();
	out.print(7, "02")(':').print(s, ".7").lf();
	out.print(6, "02")(':').print(s, ".6").lf();
	out.print(5, "02")(':').print(s, ".5").lf();
	out.print(4, "02")(':').print(s, ".4").lf();
	out.print(3, "02")(':').print(s, ".3").lf();
	out.print(2, "02")(':').print(s, ".2").lf();
	out.print(1, "02")(':').print(s, ".1").lf();
}

static void test(sizeval_t s) {
	out.print(s).lf();
	out.print(s, ".").lf();
}

extern "C" int main() {
	test(sizeval::unit);
	test(sizeval::kilo);
	test(sizeval::kilo * 2);
	test(sizeval::mega);
	test(sizeval::giga);
	test(sizeval::tera);
	test(sizeval::peta);
	test(sizeval::peta * 2);
	test(sizeval::exa);
	test(4 * sizeval::exa - 1);

	test(sizeval::unlimited);
	test(sizeval::unlimited - 1);
	test(sizeval::unlimited - sizeval::kilo + 1);
	test(sizeval::unlimited / 2 + 1);

	test(sizeval_t(765376126));

	test_prec(sizeval::kilo + 2);
	test_prec(sizeval::kilo + sizeval::kilo / 2);
	test_prec(sizeval::kilo * 2 + 2);

	return 0;
}
