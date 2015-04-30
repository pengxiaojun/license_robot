MAKE := $(MAKE) -s

ifndef NOAUTOSRC
CSRC += $(wildcard *.c)
CSRC := $(strip $(subst dllmain.c,,$(CSRC)))
CSRC := $(strip $(subst $(CEXCLUDE),,$(CSRC)))
CXXSRC += $(wildcard *.cpp)
CXXSRC := $(strip $(subst dllmain.cpp,,$(CXXSRC)))
CXXSRC := $(strip $(subst $(CXXEXCLUDE),,$(CXXSRC)))

CSRC += $(foreach DIR, $(SRCDIRS), $(wildcard $(DIR)/*.c))
CXXSRC += $(foreach DIR, $(SRCDIRS), $(wildcard $(DIR)/*.cpp))
endif

COBJ = $(patsubst %.c, objs/%.o, $(CSRC))
CXXOBJ = $(patsubst %.cpp, objs/%.o, $(CXXSRC))

OBJS = $(COBJ) $(CXXOBJ)

#ifdef DEBUG
CFLAGS += -ggdb
CXXFLAGS += -ggdb
#endif

PROJROOT = .
TAGDIR = sharedinc
ifneq ($(findstring $(TAGDIR),$(wildcard $(TAGDIR))),)
PROJROOT = .
else ifneq ($(findstring $(TAGDIR), $(wildcard ../$(TAGDIR))),)
PROJROOT = ..
else ifneq ($(findstring $(TAGDIR), $(wildcard ../../$(TAGDIR))),)
PROJROOT = ../..
else ifneq ($(findstring $(TAGDIR), $(wildcard ../../../$(TAGDIR))),)
PROJROOT = ../../..
else ifneq ($(findstring $(TAGDIR), $(wildcard ../../../../$(TAGDIR))),)
PROJROOT = ../../../..
endif

ARCH = $(shell $(PROJROOT)/arch.sh)
ifeq (x86, $(ARCH))
OUTDIR = $(PROJROOT)/output
else ifeq (x64, $(ARCH))
OUTDIR = $(PROJROOT)/output64
else
OUTDIR = $(PROJROOT)/output
endif

$(shell [ -d $(OUTDIR) ] || mkdir $(OUTDIR))
$(shell [ -d objs ] || mkdir objs)

CFLAGS += -Wall -I$(PROJROOT)/sharedinc
CXXFLAGS += -Wall -I$(PROJROOT)/sharedinc

CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE
CXXFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE

CXXFLAGS += -std=c++1y

LDFLAGS += -luuid -lrt
LDFLAGS += -L$(OUTDIR) -Wl,-rpath=./,-rpath=./modules

ifdef LIBS
LDFLAGS += $(addprefix -l,$(LIBS))
endif

ifdef BINTARGET
$(BINTARGET): $(OBJS)
	@echo "Linking $@"; $(CXX) $^ $(LDFLAGS) -o $@
	@test -d $(OUTDIR) && cp $@ $(OUTDIR) || test -f $@
endif

ifdef SOTARGET
CFLAGS += -fPIC
CXXFLAGS += -fPIC
LDFLAGS += -shared -Wl,-soname=$(SOTARGET)
IS_SO = 1
ifeq ($(findstring .so, $(SOTARGET)),)
IS_SO = 0
endif
$(SOTARGET): $(OBJS)
	@echo "Linking $@"; $(CXX) $^ $(LDFLAGS) -o $@
	@test -d $(OUTDIR) && test $(IS_SO) -eq 1 && cp $@ $(OUTDIR) || test -f $@
	@test -d ../plugins && test $(IS_SO) -eq 0 && cp $@ ../plugins || test -f $@
endif

ifdef LIBTARGET
$(LIBTARGET): $(OBJS)
	$(AR) rcs $@ $^
	@test -d $(OUTDIR) && cp $@ $(OUTDIR) || test -f $@
endif

.PHONY: clean debug release help

clean:
	@$(RM) $(OBJS) $(BINTARGET) $(SOTARGET) $(LIBTARGET)

rebuild:
	@$(MAKE) -j 4 $(TARGET) clean
	@$(MAKE) -j 4 $(TARGET)

rb:
	@$(MAKE) rebuild

debug:
	@$(MAKE) -j 4 $(TARGET) DEBUG=1

release:
	@$(MAKE) -j 4 $(TARGET)

help:
	@echo "make [clean|debug|release|help]"

objs/%.o: %.c
	@[ -d $(dir $@) ] || mkdir $(dir $@)
	@echo "Compiling $<"; $(CC) $(CFLAGS) -c $< -o $@

objs/%.o: %.cpp
	@[ -d $(dir $@) ] || mkdir $(dir $@)
	@echo "Compiling $<"; $(CXX) $(CXXFLAGS) -c $< -o $@
