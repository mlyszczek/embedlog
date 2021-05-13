[kursg-meta]: # (order: 1)

About
=====

Logger written in **C89** targeting embedded systems. It's main goals are to be
small and fast. Many features can be disabled during compilation to generate
smaller binary. Amount of needed stack memory can be lowered by adjusting
maximum printable size of single log message. Altough logger focuses on embedded
systems, it is general **c/c++** logger that can be used in any application.
Implemented features are (most of them optionally configured in runtime):

* printing to different outputs (simultaneously) like:
    * syslog (very limited, works on *nuttx* for now)
    * directly to serial device (like /dev/ttyS0)
    * standard error (stderr)
    * standard output (stdout)
    * file (with optional log rotate, and syncing to prevent data loss)
    * automatic file reopening on unexpected events (file deletion, SD remount)
    * custom routine - can be anything **embedlog** just calls custom function
      with string to print
* appending timestamp to every message
    * clock_t
    * time_t
    * CLOCK_REALTIME (requires POSIX)
    * CLOCK_MONOTONIC (requires POSIX)
    * configurable precision of fraction of seconds (mili, micro, nano)
* print location of printed log (file and line)
* print function name of printed log (required at least c99)
* 8 predefinied log levels (total rip off from syslog(2))
* colorful output (ansi colors) for easy error spotting
* print memory block in wireshark-like output
* fully binary logs with binary data (like CAN frames) to save space
* full thread safety using *pthread*

Almost all of these features can be disabled to save some precious bytes of
memory.

Functions description
=====================

For detailed description of every function check out
[man pages](https://embedlog.bofc.pl/manuals.html)

Examples
========

Check [examples](https://git.bofc.pl/embedlog/tree/examples) directory to get
the idea of how to use **embedlog**. Examples can also be compiled to see how
they work.

Dependencies
============

Library is written in **C89** but some features require implemented **POSIX** to
work. Also there are some additional features for users with **C99** compiler.

To run unit tests, you also need [librb](https://librb.bofc.pl)

Test results
============

Project has been tested on following systems/architectures

operating system tests
----------------------

* arm-cortex-m4-nuttx-7.24 (manual)
* parisc-polarhome-hpux-11.11
* power4-polarhome-aix-7.1
* i686-builder-freebsd-11.1
* i686-builder-netbsd-8.0
* i686-builder-openbsd-6.2
* x86_64-builder-solaris-11.3
* i686-builder-linux-gnu-4.9
* i686-builder-linux-musl-4.9
* i686-builder-linux-uclibc-4.9
* x86_64-builder-linux-gnu-4.9
* x86_64-builder-linux-musl-4.9
* x86_64-builder-linux-uclibc-4.9
* x86_64-bofc-debian-9
* x86_64-bofc-centos-7
* x86_64-bofc-fedora-28
* x86_64-bofc-opensuse-15
* x86_64-bofc-rhel-7
* x86_64-bofc-slackware-14.2
* x86_64-bofc-ubuntu-18.04

machine tests
-------------

* aarch64-builder-linux-gnu
* armv5te926-builder-linux-gnueabihf
* armv6j1136-builder-linux-gnueabihf
* armv7a15-builder-linux-gnueabihf
* armv7a9-builder-linux-gnueabihf
* mips-builder-linux-gnu

sanitizers
----------

* -fsanitize=address
* -fsanitize=leak
* -fsanitize=undefined

Compiling and installing
========================

Compiling library
-----------------

Project uses standard automake so to build you need to:

~~~
$ autoreconf -i
$ ./configure
$ make
# make install
~~~

Running tests
-------------

To run test simply run

~~~
$ make check
~~~

Compiling examples
------------------

Compile examples with

~~~
$ cd examples
$ make
~~~

Build time options
==================

Many features can be disabled to save space and ram. While this may not be
neccessary to change on big operating systems such as **linux** or **freebsd**,
it may come in handy when compiling for very small embedded systems. All options
are passed to configure script in common way **./configure --enable-_feature_**.
Run **./configure --help** to see help on that matter. For all **--enable**
options it is also valid to pass **--disable**. Enabling option here does not
mean it will be hard enabled in runtime, this will just give you an option to
enable these settings later in runtime.

--enable-out-stderr (default: enable)
-------------------------------------

When set, library will be able to print logs to standard error output (stderr)
and standard output (stdout). Nothing fancy.

--enable-out-file (default: enable)
-----------------------------------

Allows to configure logger to print logs to file. Optional file rotation can be
enabled. Number of rotation files and maximum size of rotation log file can be
defined in runtime

--enable-out-custom (default: enable)
-------------------------------------

Allows to pas own function which will receive fully constructed message to print
as **const char \***. Usefull when there is no output facility that suits your
needs.

--enable-timestamp (default: enable)
------------------------------------

When enabled, logger will be able to add timestamp to every message. Timestamp
can be in short or long format and timer source can be configured. Check out
[man page](https://embedlog.bofc.pl/manuals/el_option.3.html) to read more
about it.

--enable-fractions (default: enable)
------------------------------------

When enabled, logger will be able to add fractions of seconds to each message.
Fractions are added after reguler timestamp in format ".mmm" where mmm is
fractions of seconds in milliseconds. This can be tuned to use micro or even
nanoseconds - if system has such resolution.

--enable-realtime, --enable-monotonic (default: enable)
-------------------------------------------------------

Allows to use better precision timers - **CLOCK_REALTIME** and
**CLOCK_MONOTONIC** but requires **POSIX**

--enable-clock (default: enable)
--------------------------------

Allows logger to use clock(3) as time source

--enable-binary-logs (default: disable)
---------------------------------------

This will allow you to log binary data (like data read from CAN). Such logs
cannot be read with ordinary *cat* or *less* and will ned custom-made log
decoder, but such logs will use much less space on block devices. This of
course can be used with file rotation. This doesn't work with *stderr* or
*syslog* output as it would make no sense to send binary data there

--enable-prefix (default: enable)
---------------------------------

This will allow user to add custom string prefix to each message printed.
Very usefull when multiple programs logs to single source (like *syslog* or
*stderr*, it's easier to distinguish who sent that log. It's also usefull
when you want to merge logs from multiple files into on big file of logs.

--enable-finfo (default: enable)
--------------------------------

When enabled, information about line and file name from where log originated
will be added to each message.

--enable-funcinfo (default: disable)
------------------------------------

When enabled, information about function name from where log originated
will be added to each message. This uses *__func__* so you need compiler
that supports that. It was added in *c99* standard.

--enable-colors (default: enable)
---------------------------------

If enabled, output logs can be colored depending on their level. Good for
quick error spotting.

--enable-colors-extended (default: disable)
-------------------------------------------

When enable, *embedlog* will use more colors for some log levels. Without that
some log levels will have same output color. Not all terminals/tools supports
extended colors.

--enable-reentrant (default: enable)
------------------------------------

Uses reentrant functions where possible. Not available on every platform, but
if enabled, provides thread-safety on line level - that means, lines won't
overlap with another thread. This is true only when output is *stderr* or
*stdout*, when output is *file*, you need to use true thread safety with
the help of **EL_THREAD_SAFE** and **--enable-pthread**.

--enable-pthread (default: enable)
----------------------------------

 When enabled, you will be able to configure **embedlog** to use
**EL_THREAD_SAFE**, which will provide full thread safety in all circumstances.
This is critical if output is other than *stderr* or *stdout* - like *file*,
as there is internal state in *el* object that is kept between calls.

--enable-portable-snprintf (default: disable)
---------------------------------------------

When enabled, library will use internal implementation of **snprintf** even if
**snprintf** is provided by the operating system.

Contact
=======

Michał Łyszczek <michal.lyszczek@bofc.pl>

License
=======

Library is licensed under BSD 2-clause license. See
[LICENSE](https://git.bofc.pl/embedlog/tree/LICENSE) file for details

See also
========

* [c89 snprintf function family](https://www.ijs.si/software/snprintf) by
  Mark Martinec
* [mtest](https://mtest.bofc.pl) unit test framework **embedlog** uses
* [librb](https://librb.bofc.pl) ring buffer used in unit tests
* [git repository](https://git.bofc.pl/embedlog) to browse code online
* [polarhome](http://www.polarhome.com) nearly free shell accounts for virtually
  any unix there is.
* [pvs studio](https://www.viva64.com/en/pvs-studio) static code analyzer with
  free licenses for open source projects
