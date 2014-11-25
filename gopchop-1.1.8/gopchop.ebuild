# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit eutils

DESCRIPTION="MPEG2 GOP-accurate editor."
HOMEPAGE="http://outflux.net/unix/software/GOPchop/"
SRC_URI="mirror://sourceforge/gopchop/${P}.tar.gz"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"
IUSE="sdl"

DEPEND="
sdl? ( media-libs/libsdl )
x11-libs/gtk+
>=media-libs/libmpeg2-0.4.0
"

src_compile() {
	local myconf
	use sdl || myconf="${myconf} --disable-sdl"
	econf ${myconf} || die "Configuration failed."
	emake || die "Make failed."
}

src_install() {
	einstall || die
}
src_unpack() {
	unpack ${A}
}
