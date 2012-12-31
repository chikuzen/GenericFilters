include config.mak

vpath %.c $(SRCDIR)
vpath %.h $(SRCDIR)

SRCS = convolution/convolution.c  convolution/proc_3x3_sse2.c convolution/proc_5x5_sse2.c \
       convolution/proc_h_sse2.c  convolution/proc_v_sse2.c   convolution/convolution_hv.c \
       convolution/proc_hv_sse2.c convolution/xxflate.c       convolution/inflate.c \
       convolution/deflate.c      neighbors/maximum_sse2.c    neighbors/median_sse2.c \
       neighbors/minimum_sse2.c   neighbors/neighbors.c       alone/invert.c \
       alone/limitter.c           alone/levels.c              alone/binarize.c \
       alone/alone.c              common/common.c

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
	@$(foreach SRC, $(SRCS:%=$(SRCDIR)/%), $(CC) $(SRC) $(CFLAGS) -msse2 -MT $(SRC:$(SRCDIR)/%.c=%.o) -MM >> .depend;)

config.mak:
	./configure
