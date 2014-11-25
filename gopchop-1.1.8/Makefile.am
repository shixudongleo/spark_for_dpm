## Process this file with automake to produce Makefile.in

SUBDIRS = libvo src po include docs pixmaps tests

EXTRA_DIST = \
    autogen.sh \
    gopchop.glade \
    gopchop.gladep \
    gopchop.spec \
    gopchop.desktop \
    gopchop.ebuild \
    intltool-extract.in intltool-merge.in intltool-update.in

# I think this should be considered an intltool bug
DISTCLEANFILES = \
                 intltool-extract \
                 intltool-merge \
                 intltool-update

appsdir = $(datadir)/applications
apps_DATA = gopchop.desktop

gopchop.spec: config.status
