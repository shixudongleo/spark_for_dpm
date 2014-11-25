## Process this file with automake to produce Makefile.in

# I want to do something like this...(so that .cpp is checked before .c)
#.SUFFIXES: .cpp .c .o

INCLUDES = \
        -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
        -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	@INCLUDES@

bin_PROGRAMS = gopchop mpegcat gtkspu
EXTRA_PROGRAMS = m2parse

gopchop_SOURCES = \
	Main.cpp main.h \
	support.c support.h \
	interface.c interface.h \
	mpeg2consts.c mpeg2consts.h \
	callbacks.cpp callbacks.h	\
	rc.c rc.h \
	display.h display.cpp \
	GOPutils.cpp GOPutils.h \
	Vector.cpp Vector.h \
	Bounds.cpp Bounds.h \
	Pack.cpp Pack.h \
	ElementStream.cpp ElementStream.h \
	GroupOfPictures.cpp GroupOfPictures.h \
	List.cpp List.h \
	Picture.cpp Picture.h \
	Parser.cpp Parser.h \
	FirstPack.cpp FirstPack.h \
	widgets.h widgets.cpp \
	sessions.h sessions.cpp \
	MPEG2Parser.cpp MPEG2Parser.h mpeg2structs.h

gopchop_LDADD = @PACKAGE_LIBS@ \
        $(top_builddir)/libvo/libvo.a @LIBVO_LIBS@ \
	$(INTLLIBS)

m2parse_SOURCES = \
	m2parse.cpp \
	Vector.cpp Vector.h \
	Bounds.cpp Bounds.h \
	Pack.cpp Pack.h \
	ElementStream.cpp ElementStream.h \
	GroupOfPictures.cpp GroupOfPictures.h \
	List.cpp List.h \
	Picture.cpp Picture.h \
	Parser.cpp Parser.h \
	MPEG2Parser.cpp MPEG2Parser.h mpeg2structs.h \
	FirstPack.cpp FirstPack.h

mpegcat_SOURCES = \
	mpeg2consts.c mpeg2consts.h \
	file_buffer.c file_buffer.h \
	mpegcat.c mpeg2structs.h

gtkspu_SOURCES = \
	file_buffer.c file_buffer.h \
	gtkspu.c
gtkspu_LDADD = @PACKAGE_LIBS@ \
	$(INTLLIBS)
