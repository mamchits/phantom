# MODULE,dir,package_suffix,local_libs,nonlocal_libs

define MODULE

mod_target.$(1) = lib/phantom/mod_$(subst /,_,$(1)).so

mod_srcs.$(1) = $(shell find phantom/$(1) -maxdepth 1 -name "*.C" | sort)
mod_objs.$(1) = $$(mod_srcs.$(1):%.C=%.s.o)

ifneq ($(3),)
mod_slibs.$(1) = $(3:%=lib/libpd-%.s.a)
mod_link.$(1) = -Wl,--whole-archive $$(mod_slibs.$(1)) -Wl,--no-whole-archive
endif

ifneq ($(4),)
mod_ext_link.$(1) = $(4:%=-l%)
endif

$$(mod_target.$(1)): $$(mod_objs.$(1)) $$(mod_slibs.$(1))
	$$(CXX) -shared $$(mod_objs.$(1)) $$(mod_link.$(1)) $$(mod_ext_link.$(1)) -o $$(@)

dirs_pd$(2) += phantom/$(1)
modules$(2) += $$(mod_target.$(1))

tmps_phantom_module += $$(mod_objs.$(1))
targets_phantom_module += $$(mod_target.$(1))

tmps += $$(mod_objs.$(1))
targets += $$(mod_target.$(1))

endef
