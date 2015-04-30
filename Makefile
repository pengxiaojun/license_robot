LIBTARGET = libgrlic.a

CFLAGS += $(GRUTIL_INCS) -fPIC
CXXFLAGS += $(GRUTIL_INCS) -fPIC

include Makefile.inc
