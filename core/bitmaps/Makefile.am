EXTRA_DIST = genxpm.sh $(srcdir)/*.bmp $(srcdir)/*.ico
CLEANFILES = Makefile.bitmaps *.xpm

.PHONY: all-bitmaps

Makefile.bitmaps: $(srcdir)/genxpm.sh $(top_srcdir)/res.rc
	/bin/sh $^ $(srcdir) >$@

all: all-bitmaps

all-bitmaps: Makefile.bitmaps
	make -f Makefile.bitmaps all


