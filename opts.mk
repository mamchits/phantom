ifneq ($(SUFFIXES),)
$(error missing -R make option)
endif

CXX = g++
CC = gcc

OPT ?= 3

CXXSTD = -std=gnu++0x

define FIXZERODEPS
$(subst %,/,$(1:.d=)): force_fixzerodeps

force:;

.PHONY: force_fixzerodeps
endef

$(eval $(call FIXZERODEPS,$(shell find deps/ -size 0 -printf "%P ")))


FIXINC ?= -isystem . -isystem /usr/include/pd/fixinclude

CPPFLAGS = \
	$(FIXINC) -D_GNU_SOURCE=1 $(CPPDEFS) -Wundef

DEPS = \
	-MD -MF deps/$(subst /,%,$(@)).d

CXXFLAGS = \
	-g -O$(OPT) $(CXXSTD) \
	-fvisibility=hidden -fvisibility-inlines-hidden -fno-default-inline \
	-fno-omit-frame-pointer -fno-common -fsigned-char \
	-Wall -W -Werror -Wsign-promo -Woverloaded-virtual \
	-Wno-ctor-dtor-privacy -Wno-non-virtual-dtor $(CPPFLAGS) $(CXXFLAGS.$(<))

CFLAGS = \
	-g -O$(OPT) $(CSTD) \
	-fvisibility=hidden -fno-omit-frame-pointer -fno-common -fsigned-char \
	-Wall -W -Werror $(CPPFLAGS) $(CFLAGS.$(<))


%.o: %.C; $(CXX) -c $(CXXFLAGS) $(DEPS) $(<) -o $(@)
%.o: %.c; $(CC) -c $(CFLAGS) $(DEPS) $(<) -o $(@)
%.o: %.S; $(CC) -c $(CPPFLAGS) $(DEPS) $(<) -o $(@)

%.s.o: %.C; $(CXX) -c $(CXXFLAGS) $(DEPS) -fPIC $(<) -o $(@)
%.s.o: %.c; $(CC) -c $(CFLAGS) $(DEPS) -fPIC $(<) -o $(@)
%.s.o: %.S; $(CC) -c $(CPPFLAGS) $(DEPS) -fPIC $(<) -o $(@)


%.s: %.C; $(CXX) -S $(CXXFLAGS) $(DEPS) $(<) -o $(@)
%.s: %.c; $(CC) -S $(CFLAGS) $(DEPS) $(<) -o $(@)
%.s: %.S; $(CC) -E $(CPPFLAGS) $(DEPS) $(<) -o $(@)

%.s.s: %.C; $(CXX) -S $(CXXFLAGS) $(DEPS) -fPIC $(<) -o $(@)
%.s.s: %.c; $(CC) -S $(CFLAGS) $(DEPS) -fPIC $(<) -o $(@)
%.s.s: %.S; $(CC) -E $(CPPFLAGS) $(DEPS) -fPIC $(<) -o $(@)

-include deps/*.d
