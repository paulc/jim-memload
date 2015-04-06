# Note that if cross compiling, build with:
#
#   make NOTEST=1
#
# to avoid trying to load the resulting module.
# Also note that you will need a build-host version of jimsh in the
# PATH in order to build the extension.

# Prefer jimsh in the PATH because it is more likely to be built
# for the build-host rather than the target.

ifdef NOTEST
BUILDOPTS := --notest
endif

BUILDOPTS ?= --verbose

JIM = ../jimtcl

export PATH := $(PATH):$(JIM)

all: memload.so

memload.so: memload.c MemoryModule.c  
	$(JIM)/build-jim-ext -I$(JIM) $(BUILDOPTS) $^

test:
	JIMLIB=. $(JIM)/jimsh -e 'package require memload'

clean:
	rm -f *.o *.so
