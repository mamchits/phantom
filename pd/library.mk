# LIBRARY dir,package_suffix

define LIBRARY

lib_target.$(1) = lib/libpd-$(subst /,-,$(1)).a
slib_target.$(1) = lib/libpd-$(subst /,-,$(1)).s.a

lib_csrcs.$(1) = $(shell find pd/$(1) -maxdepth 1 -name "*.C" | sort)
lib_ssrcs.$(1) = $(shell find pd/$(1) -maxdepth 1 -name "*.S" | sort)

lib_objs.$(1) = $$(lib_csrcs.$(1):%.C=%.o) $$(lib_ssrcs.$(1):%.S=%.o)
slib_objs.$(1) = $$(lib_csrcs.$(1):%.C=%.s.o) $$(lib_ssrcs.$(1):%.S=%.s.o)

$$(lib_target.$(1)): $$(lib_objs.$(1)); ar crs $$(@) $$(lib_objs.$(1))
$$(slib_target.$(1)): $$(slib_objs.$(1)); ar crs $$(@) $$(slib_objs.$(1))

dirs_pd$(2) += pd/$(1)
libs_pd$(2) += $$(lib_target.$(1)) $$(slib_target.$(1))

tmps_pd_lib += $$(lib_objs.$(1)) $$(slib_objs.$(1))
targets_pd_lib += $$(lib_target.$(1)) $$(slib_target.$(1))

tmps += $$(lib_objs.$(1)) $$(slib_objs.$(1))
targets += $$(lib_target.$(1)) $$(slib_target.$(1))

endef
