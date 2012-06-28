empty =

define PACKAGE

pkname$(1)=phantom$(subst _,-,$(1))

ifneq ($$(dirs_pd$(1)),)
headers_pd$(1) = $(shell find $(dirs_pd$(1)) -maxdepth 1 -name "*.[Hh]" -printf "%p:usr/include/%h\n")
hdirs_pd$(1) = $$(shell echo $$(headers_pd$(1)) | sed -e 's, ,\n,g' | sed 's,.*:,/,' | sort | uniq)
endif

utilities$(1) += $(empty)
modules$(1) += $(empty)
others_pd$(1) += $(empty)
libs_pd$(1) += $(empty)
hdirs_pd$(1) += $(empty)
headers_pd$(1) += $(empty)

debian/$$(pkname$(1)).dirs:
	@{ \
		echo -n $$(firstword $$(utilities$(1))) | sed 's,.\+,/usr/bin\n,'; \
		echo -n $$(firstword $$(modules$(1))) | sed 's,.\+,/usr/lib/phantom\n,'; \
	} >$$(@)

targets_package$(1) += debian/$$(pkname$(1)).dirs

debian/$$(pkname$(1)).install:
	@{ \
		echo -n "$$(utilities$(1))" | sed 's, , usr/bin\n,g'; \
		echo -n "$$(modules$(1))" | sed 's, , usr/lib/phantom\n,g'; \
	} >$$(@)

targets_package$(1) += debian/$$(pkname$(1)).install

debian/$$(pkname$(1))-dev.dirs:
	@{ \
		echo -n $$(firstword $$(libs_pd$(1))) | sed 's,.\+,/usr/lib\n,'; \
		echo -n "$$(hdirs_pd$(1))" | sed 's, ,\n,g'; \
		echo -n $$(firstword $$(others_pd$(1))) | sed 's,.\+,/usr/share/phantom\n,'; \
	} >$$(@)

targets_package$(1) += debian/$$(pkname$(1))-dev.dirs

debian/$$(pkname$(1))-dev.install:
	@{ \
		echo -n "$$(libs_pd$(1))" | sed 's, , usr/lib\n,g'; \
		echo -n "$$(headers_pd$(1))" | sed 's, ,\n,g' | sed 's,:, ,'; \
		echo -n "$$(others_pd$(1))" | sed 's, , usr/share/phantom\n,g'; \
	} >$$(@)

targets_package$(1) += debian/$$(pkname$(1))-dev.install

targets_debian += $$(targets_package$(1))
targets += $$(targets_package$(1))

endef
