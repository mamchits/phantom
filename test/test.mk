# TEST dir,deps

define TEST

test_srcs.$(1) = $(shell find test/$(1) -maxdepth 1 -name "*.C" | sort)

ifneq ($$(test_srcs.$(1)),)

test_objs.$(1) = $$(test_srcs.$(1):%.C=%.o)
tests.$(1) = $$(test_srcs.$(1):%.C=%)

ifneq ($(2),)
test_libs.$(1) = $(2:%=lib/libpd-%.a)
endif

$$(tests.$(1)): % : %.o
	$$(CXX) $(LFLAGS.$(@)) $$(<) lib/libpd-$(1).a $$(test_libs.$(1)) -ldl -lpthread -o $$(@)

tmps_test += $$(test_objs.$(1))
targets_test += $$(tests.$(1))

tmps += $$(test_objs.$(1))
targets += $$(tests.$(1))

# ===

test_res.$(1) = $$(tests.$(1):%=%.res)

$$(test_res.$(1)): %.res : % %.pat
	@($$(<) > $$(@) && diff -au $$(word 2,$$(^)) $$(@) >&2) || (rm $$(@) && false)

targets += $$(test_res.$(1)) test_report.$(1)

test_report.$(1): $$(test_res.$(1))
	@echo all tests in test/$(1) ok

.PHONY: test_report.$(1)

endif

endef
