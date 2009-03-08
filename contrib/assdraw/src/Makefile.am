SUFFIXES = .cpp

SUBDIRS = wxAGG xpm

AM_CXXFLAGS = @WX_CPPFLAGS@ @LIBAGG_CFLAGS@ -I../libpropgrid/include

bin_PROGRAMS = assdraw
#assdraw_CPPFLAGS =
assdraw_LDFLAGS = -L../libpropgrid -lpropgrid @WX_LIBS@ @LIBAGG_LIBS@
assdraw_LDADD = wxAGG/libaggwindow.a xpm/libres.a

assdraw_SOURCES = \
	wxAGG/AGGWindow.cpp \
	agg_bcspline.cpp \
	agg_vcgen_bcspline.cpp \
	assdraw.cpp \
	assdraw_settings.cpp \
	canvas.cpp \
	canvas_mouse.cpp \
	cmd.cpp \
	dlgctrl.cpp \
	engine.cpp \
	library.cpp \
	settings.cpp

assdraw_SOURCES += \
	agg_bcspline.h \
	agg_conv_bcspline.h \
	agg_vcgen_bcspline.h \
	resource.h \
	wxAGG/AGGWindow.h \
	wxAGG/PixelFormatConvertor.h

EXTRA_DIST = \
	_common.hpp \
	agghelper.hpp 
	assdraw.hpp \
	canvas.hpp \
	canvas_mouse.hpp \
	cmd.hpp \
	dlgctrl.hpp \
	engine.hpp \
	enums.hpp \
	include_once.hpp \
	library.hpp \
	settings.hpp
