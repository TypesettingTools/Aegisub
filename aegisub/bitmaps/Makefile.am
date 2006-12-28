EXTRA_DIST = genxpm.sh $(srcdir)/*.bmp $(srcdir)/*.ico
CLEANFILES = Makefile.bitmaps *.xpm

.PHONY: all-bitmaps

Makefile.bitmaps: $(srcdir)/genxpm.sh $(srcdir)/../res.rc
	/bin/sh $^ $(srcdir) >$@

all: all-bitmaps

all-bitmaps: Makefile.bitmaps
	make -f Makefile.bitmaps CONVERT=$(CONVERT) all


