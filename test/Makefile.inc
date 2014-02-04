include test/test.mk
others_pd += test/test.mk

# ===
# TEST dir,deps

$(eval $(call TEST,base))
$(eval $(call TEST,bq,base))
$(eval $(call TEST,http,base))
$(eval $(call TEST,pi,base))

CXXFLAGS.test/bq/bug_glibc_errno.C = -O3
CXXFLAGS.test/bq/align.C = -O3

$(targets_test) : test/test.mk
