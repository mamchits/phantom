default: all

include pd/Makefile.inc
include phantom/Makefile.inc
include test/Makefile.inc

-include private/Makefile.inc

# ===

ifeq ($(OPT),0)
CPPDEFS += -D DEBUG_ALLOC
endif

FIXINC = -isystem . -isystem ./pd/fixinclude

include opts.mk
others_pd += opts.mk

# ===

include debian/Makefile.inc

# ===

all: $(targets)

clean:; @rm -f $(targets) $(tmps) deps/*.d

.PHONY: default all clean
