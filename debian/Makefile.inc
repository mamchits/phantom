include debian/package.mk
others_pd += debian/package.mk

$(eval $(call PACKAGE))
$(eval $(call PACKAGE,_ssl))
$(eval $(call PACKAGE,_debug))

debian/phantom.examples:
	@ls -1 examples/*.conf >$(@)

targets_debian += debian/phantom.examples
targets += debian/phantom.examples

$(targets_debian): debian_force

debian_force:;

.PHONY: debian_force
