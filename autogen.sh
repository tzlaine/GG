#!/bin/sh
echo "** Creating configure and friends"
set -x
aclocal -I m4
autoconf
libtoolize
autoheader
automake -a --foreign
set +x
if test ! -f configure || test ! -f ltmain.sh || test ! -f config.h.in || test ! -f Makefile.in; then
   cat<<EOT
** Unable to generate all required files!
** you'll need autoconf 2.5, automake 1.7, libtool 1.5, autoheader and aclocal installed
EOT
   exit 1
fi
echo "Now run ./configure, see ./configure --help for more information"

   
