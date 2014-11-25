#!/bin/sh
# Run this to generate all the initial makefiles, etc.

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo \`$0\'" command line."
  echo
fi

AUTOCONF_REQUIRED_VERSION=2.52
AUTOMAKE_REQUIRED_VERSION=1.7
GLIB_REQUIRED_VERSION=2.0.0
INTLTOOL_REQUIRED_VERSION=0.22

check_version ()
{
    if expr $1 \>= $2 > /dev/null; then
        echo "yes (version $1)"
    else
        echo "Too old (found version $1)!"
        DIE=1
    fi
}

attempt_command () {
    IGNORE=$1
    shift

    echo "Running $@ ..."
    ERR="`$@ 2>&1`"
    errcode=$?
    if [ "x$IGNORE" = "x" ]; then
        ERR=`echo "$ERR"`
    else
        ERR=`echo "$ERR" | egrep -v "$IGNORE"`
    fi
    if [ "x$ERR" != "x" ]; then
        echo "$ERR" | awk '{print "  " $0}'
    fi
    if [ $errcode -gt 0 ]; then
        echo "Please fix the error conditions and try again."
        exit 1
    fi
}

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

DIE=0

if [ -n "$GNOME2_DIR" ]; then
	ACLOCAL_FLAGS="-I $GNOME2_DIR/share/aclocal $ACLOCAL_FLAGS"
	LD_LIBRARY_PATH="$GNOME2_DIR/lib:$LD_LIBRARY_PATH"
	PATH="$GNOME2_DIR/bin:$PATH"
	export PATH
	export LD_LIBRARY_PATH
fi

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level package directory"
    exit 1
}

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

(grep "^AC_PROG_INTLTOOL" $srcdir/configure.ac >/dev/null) && {
 echo -n "checking for intltool >= $INTLTOOL_REQUIRED_VERSION ... "
 if (intltoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`intltoolize --version \
         | grep intltoolize | sed "s/.* \([0-9.]*\)/\1/"`
    check_version $VER $INTLTOOL_REQUIRED_VERSION
 else
    echo
    echo "  You must have intltool installed to compile $PROJECT."
    echo "  Get the latest version from"
    echo "  ftp://ftp.gnome.org/pub/GNOME/sources/intltool/"
    DIE=1
 fi
}

(grep "^AM_PROG_XML_I18N_TOOLS" $srcdir/configure.ac >/dev/null) && {
  (xml-i18n-toolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`xml-i18n-toolize' installed."
    echo "You can get it from:"
    echo "  ftp://ftp.gnome.org/pub/GNOME/"
    DIE=1
  }
}

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
  (libtool --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed."
    echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
  }
}

(grep "^AM_GLIB_GNU_GETTEXT" $srcdir/configure.ac >/dev/null) && {
  (grep "sed.*POTFILES" $srcdir/configure.ac) > /dev/null || \
  (glib-gettextize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`glib' installed."
    echo "You can get it from: ftp://ftp.gtk.org/pub/gtk"
    DIE=1
  }
}

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
# Prefer earlier versions just so that the earliest supported version gets test coverage by developers.
if (automake-1.7 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.7
   ACLOCAL=aclocal-1.7
elif (automake-1.8 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.8
   ACLOCAL=aclocal-1.8
elif (automake --version) < /dev/null > /dev/null 2>&1; then
   # Leave unversioned automake for a last resort: it may be a version earlier
   # than what we require.
   # (In particular, it might mean automake 1.4: that version didn't default to
   #  installing a versioned name.)
   AUTOMAKE=automake
   ACLOCAL=aclocal
else
    echo
    echo "  You must have automake 1.7 or newer installed to compile $PROJECT."
    echo "  Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.8.5.tar.gz"
    echo "  (or a newer version of 1.8 if it is available; note that 1.9 is bugg
y)"
    DIE=1
    NO_AUTOMAKE=1
fi
if test x$AUTOMAKE != x; then
    VER=`$AUTOMAKE --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
fi

# if no automake, don't bother testing for aclocal
test x$AUTOMAKE = x || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "You can get automake from ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

for coin in `find $srcdir -path $srcdir/CVS -prune -o -name configure.ac -print`
do 
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo processing $dr
    ( cd $dr

      aclocalinclude="$ACLOCAL_FLAGS"

	if grep "^AM_GLIB_GNU_GETTEXT" configure.ac >/dev/null; then
		echo "Creating $dr/aclocal.m4 ..."
		test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
#		echo "Running glib-gettextize...  Ignore non-fatal messages."
		attempt_command '^(Please add the files|  codeset|  progtest|from the|or directly|You will also|ftp://ftp.gnu.org|$)' \
			glib-gettextize --copy --force
		#echo "no" | glib-gettextize --force --copy
		echo "Making $dr/aclocal.m4 writable ..."
		test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4
	fi
	if grep "^AC_PROG_INTLTOOL" configure.ac >/dev/null; then
		attempt_command '' intltoolize --copy --force --automake
	fi
	if grep "^AM_PROG_XML_I18N_TOOLS" configure.ac >/dev/null; then
		attempt_command '' xml-i18n-toolize --copy --force --automake
	fi
	if grep "^AM_PROG_LIBTOOL" configure.ac >/dev/null; then
		if test -z "$NO_LIBTOOLIZE" ; then 
			attempt_command '' libtoolize --force --copy
		fi
	fi
	attempt_command 'underquoted definition of|[\)\#]Extending' \
		$ACLOCAL $aclocalinclude
	if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
		attempt_command '' autoheader
	fi
	attempt_command '' $AUTOMAKE --add-missing --gnu $am_opt
	attempt_command '' autoconf
    )
  fi
done

conf_flags="--enable-maintainer-mode"

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile. || exit 1
else
  echo Skipping configure process.
fi
