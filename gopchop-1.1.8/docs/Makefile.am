EXTRA_DIST = README libmpeg2.txt libvo.txt \
		gopchop.sgml gopchop.1 \
		mpegcat.sgml mpegcat.1 \
		gtkspu.sgml gtkspu.1

# TODO: figure out how "jw" can write these temp files somewhere else
DISTCLEANFILES = manpage.refs manpage.links

man_MANS = gopchop.1 mpegcat.1 gtkspu.1

%.1: %.sgml
	jw -b man $<

%.html: %.sgml
	jw -b html $< && mv index.html $@
