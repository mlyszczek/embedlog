#!/bin/sh

if [ "`uname -s`" = "SunOS" ]
then
    # autoreconf on solaris can't find m4 files,
    # so we install it manually
    aclocal -I/usr/share/aclocal --install
fi

autoreconf -fi
