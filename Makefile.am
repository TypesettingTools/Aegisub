AUTOMAKE_OPTIONS = foreign
SUBDIRS = ac FexTrackerSource csri aegisub locale

lua51/src/liblua.a:
	make -C lua51/src CC=$(CXX) a

all-recursive: lua51/src/liblua.a

