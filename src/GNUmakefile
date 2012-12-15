include config.mak

SRCS = convolution/convolution.c convolution/proc_3x3.c convolution/proc_5x5.c \
       convolution/proc_h.c convolution/proc_v.c neighbors/maximum.c \
       neighbors/median.c neighbors/minimum.c neighbors/neighbors.c \
       vapoursynth.c

OBJS = $(SRCS:%.c=%.o)

.PHONY: all clean distclean

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
	$(if $(STRIP), $(STRIP) -x $@)

%.o: %.c .depend
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJS) *.dll *.so .depend

distclean: clean
	$(RM) config.mak

ifneq ($(wildcard .depend),)
include .depend
endif

.depend: config.mak
	@$(RM) .depend
	@$(foreach SRC, $(SRCS), $(CC) $(SRC) $(CFLAGS) -g0 -MT $(SRC:%.c=%.o) -MM >> .depend;)

config.mak:
	./configure
