dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.53)
AC_INIT(gopchop, 1.1.8, gopchop-bugs@outflux.net, gopchop)
AC_CONFIG_SRCDIR(src/Main.cpp)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(include/config.h)
AM_MAINTAINER_MODE

AC_PROG_INTLTOOL(0.22)

AC_CANONICAL_HOST
AC_LANG(C++)
AC_ISC_POSIX
AM_PROG_CC_STDC

AC_HEADER_STDC
AC_SYS_LARGEFILE
AC_TYPE_OFF_T
AC_FUNC_FSEEKO

AM_PROG_LIBTOOL

AM_PATH_XML2(2.6.6,,AC_MSG_ERROR([
*** Libxml is required to build gopchop; Available at
http://www.libxml.org/.]))

AC_CACHE_SAVE

dnl AC_DEFUN([adl_FUNC_GETOPT_LONG],
dnl  [# clean out junk possibly left behind by a previous configuration
dnl   rm -f lib/getopt.h
dnl   # Check for getopt_long support
dnl   AC_CHECK_HEADERS([getopt.h])
dnl   AC_CHECK_FUNCS([getopt_long],,
dnl    [# FreeBSD has a gnugetopt library for this
dnl     AC_CHECK_LIB([gnugetopt],[getopt_long],[AC_DEFINE([HAVE_GETOPT_LONG])],
dnl      [# use the GNU replacement
dnl       AC_LIBOBJ(getopt)
dnl       AC_LIBOBJ(getopt1)
dnl       AC_CONFIG_LINKS([lib/getopt.h:lib/gnugetopt.h])])])])
dnl 
dnl dnl Stupid missing functions
dnl dnl AC_CONFIG_LIBOBJ_DIR(lib)
dnl adl_FUNC_GETOPT_LONG

dnl deal with internal includes
INCLUDES='-I$(top_srcdir)/include'
AC_SUBST([INCLUDES])

AC_CACHE_SAVE

pkg_modules="gtk+-2.0 >= 2.0.0, libxml-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

AC_CACHE_SAVE

dnl Build in libvo stuff
builtin([include],[libvo/configure.incl])

AC_CACHE_SAVE

dnl make sure mpeg2dec is available
AC_CHECK_HEADER([mpeg2dec/mpeg2.h],,
                AC_MSG_ERROR([Couldn't find header mpeg2dec/mpeg2.h!]),,)
AC_CHECK_LIB([mpeg2],[mpeg2_init],,
                AC_MSG_ERROR([Couldn't find library libmpeg2!]),,)
AC_CHECK_FUNCS([mpeg2_reset mpeg2_flush_picture],,,)

AC_CACHE_SAVE

dnl make sure mpeg2convert is available
AC_CHECK_HEADER([mpeg2dec/mpeg2convert.h],,
                AC_MSG_ERROR([Couldn't find header mpeg2dec/mpeg2convert.h!]),
		AC_INCLUDES_DEFAULT()
		[#include <mpeg2dec/mpeg2.h>])

AC_CHECK_LIB([mpeg2convert],[mpeg2convert_rgb],,
                AC_MSG_ERROR([Couldn't find library libmpeg2convert!]),,
		[$LIBVO_LIBS])

AC_CACHE_SAVE

GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],"$GETTEXT_PACKAGE",[Translation domain used])
ALL_LINGUAS="en de fr"
AM_GLIB_GNU_GETTEXT

dnl get architecture info (care-of mpeg2dec's configure.in)
if test x"$GCC" = x"yes"; then

    dnl arch-specific flags
    case "$host" in
    i?86-* | k?-* | x86_64-*)
        AC_DEFINE([ARCH_X86],,[x86 architecture])
        case "$host" in
        i386-*) OPT_CFLAGS="$CFLAGS -mcpu=i386";;
        i486-*) OPT_CFLAGS="$CFLAGS -mcpu=i486";;
        i586-*) OPT_CFLAGS="$CFLAGS -mcpu=pentium";;
        i686-*) OPT_CFLAGS="$CFLAGS -mcpu=pentiumpro";;
        k6-*)   OPT_CFLAGS="$CFLAGS -mcpu=k6";;
        esac
        AC_TRY_CFLAGS([$OPT_CFLAGS],[CFLAGS=$OPT_CFLAGS]);;
    ppc-* | powerpc-*)
        AC_CHECK_HEADER([altivec.h],
            [TRY_CFLAGS="$ARCH_OPT_CFLAGS -maltivec"],
            [TRY_CFLAGS="$ARCH_OPT_CFLAGS -faltivec"])
        AC_TRY_CFLAGS([$OPT_CFLAGS $TRY_CFLAGS $CFLAGS],
            [ARCH_OPT_CFLAGS="$TRY_CFLAGS"
            AC_DEFINE([ARCH_PPC],,[ppc architecture])]);;
    sparc-* | sparc64-*)
        AC_DEFINE([ARCH_SPARC],,[sparc architecture])
        TRY_CFLAGS="$OPT_CFLAGS -mcpu=ultrasparc -mvis"
        AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"]);;
    alpha*)
        AC_DEFINE([ARCH_ALPHA],,[alpha architecture]);;
    esac

fi

dnl Use -Wall if we have gcc.
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *\ -Wall\ *) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

AC_CONFIG_FILES([
Makefile
libvo/Makefile
include/Makefile
docs/Makefile
src/Makefile
pixmaps/Makefile
tests/Makefile
po/Makefile.in
gopchop.spec
])

AC_OUTPUT

