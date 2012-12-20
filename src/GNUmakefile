include config.mak

vpath %.c $(SRCDIR)
vpath %.h $(SRCDIR)

SRCS = convolution/convolution.c convolution/proc_3x3.c convolution/proc_5x5.c \
       convolution/proc_h.c      convolution/proc_v.c   convolution/proc_hv.c \
       neighbors/maximum.c       neighbors/median.c     neighbors/minimum.c \
       neighbors/neighbors.c     alone/invert.c         alone/limitter.c \
       alone/levels.c            common/common.c

OBJS = $(SRCS:%.c=%.o)

.PHONY: all install clean distclean dep

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $^
	-@ $(if $(STRIP), $(STRIP) $@)

%.o: %.c .depend
	@echo $(CC) -o $@ $(<:$(SRCDIR)/%.c=%.o)
	@$(CC) -c $(CFLAGS) -o $@ $<

install: all
	install -d $(libdir)
	install -m 755 $(LIBNAME) $(libdir)

clean:
	$(RM) $(OBJS) *.dll *.so *.dylib .depend

distclean: clean
	$(RM) config.*

dep: .depend

ifneq ($(wildcard .depend),)
include .depend
endif

.depend: config.mak
	@$(RM) .depend
	@$(foreach SRC, $(SRCS:%=$(SRCDIR)/%), $(CC) $(SRC) $(CFLAGS) -MT $(SRC:$(SRCDIR)/%.c=%.o) -MM >> .depend;)

config.mak:
	./configure
