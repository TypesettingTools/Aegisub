libhunspell_la_LDFLAGS = -version-info 1:1:0

lib_LTLIBRARIES = libhunspell.la
libhunspell_includedir = $(includedir)/hunspell
libhunspell_la_SOURCES=affentry.cxx affixmgr.cxx csutil.cxx \
		     dictmgr.cxx hashmgr.cxx hunspell.cxx utf_info.cxx \
	             suggestmgr.cxx license.myspell license.hunspell \
	             phonet.cxx

libhunspell_include_HEADERS=affentry.hxx htypes.hxx affixmgr.hxx \
	        csutil.hxx hunspell.hxx atypes.hxx dictmgr.hxx hunspell.h \
		suggestmgr.hxx baseaffix.hxx hashmgr.hxx langnum.hxx \
		phonet.hxx

EXTRA_DIST=hunspell.dsp makefile.mk README
