include pd/library.mk
others_pd += pd/library.mk

# ===
# LIBRARY dir,package_suffix

$(eval $(call LIBRARY,base))
$(eval $(call LIBRARY,bq))
$(eval $(call LIBRARY,http))
$(eval $(call LIBRARY,ssl,_ssl))
$(eval $(call LIBRARY,debug,_debug))

$(targets_pd_lib) : pd/library.mk

# ===

dirs_pd += pd/fixinclude

# ===

CXXFLAGS.pd/bq/test/bug_glibc_errno.C = -O3
CXXFLAGS.pd/bq/test/align.C = -O3

#bin/trace_print: util/trace_print.o
#	$(CXX) util/trace_print.o lib/libpd-debug.a lib/libpd-base.a \
#		-lbfd -ldl -lpthread -o $(@)
#
#targets += bin/trace_print
#tmps += util/trace_print.o
