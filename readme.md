[kursg-meta]: # (order: 1)

About
=====

Logger written in **C89** targeting embedded systems. It's main goals are to be
small and fast. Many features can be disabled during compilation to generate
smaller binary. Ammount of needed stack memory can be lowered by adjusting
maximum printable size of single log message. Altough logger focuses on embedded
systems, it is general **c/c++** logger that can be used in any application.
Implemented features are:

* printing to different outputs (simultaneously) like:
    * standard error (stderr)
    * file (with optional log rotate)
    * custom routine - can be anything **embedlog** just calls custom function
      with string to print
* appending timestamp to every message
    * clock_t
    * time_t
    * CLOCK_REALTIME (requires POSIX)
    * CLOCK_MONOTONIC (requires POSIX)
* print location of printed log (file and line)
* 8 predefinied log levels and (sizeof(int) - 8) custom log levels ;-)
* colorful output (ansi colors) for easy error spotting
* print memory block in wireshark-like output

Almost all of these features can be disabled to save some precious bytes of
memory.

Functions description
=====================

For detailed description of every function check out
[man pages](http://embedlog.kurwinet.pl/manuals.html)

Examples
========

Check [examples](http://git.kurwinet.pl/embedlog/tree/examples) directory to get
the idea of how to use **embedlog**. Examples can also be compiled to see how
they work.

Dependencies
============

Library is written in **C89** but some features require implemented **POSIX** to
work. Also there are some additional features for users with **C99** compiler.

To run unit tests, you also need [librb](http://librb.kurwinet.pl)

Test results
============

operating system tests
----------------------

* parisc-polarhome-hpux-11.11 ![test-result-svg][prhpux]
* power4-polarhome-aix-7.1 ![test-result-svg][p4aix]
* i686-builder-freebsd-11.1 ![test-result-svg][x32fb]
* i686-builder-netbsd-8.0 ![test-result-svg][x32nb]
* i686-builder-openbsd-6.2 ![test-result-svg][x32ob]
* x86_64-builder-solaris-11.3 ![test-result-svg][x64ss]
* i686-builder-linux-gnu-4.9 ![test-result-svg][x32lg]
* i686-builder-linux-musl-4.9 ![test-result-svg][x32lm]
* i686-builder-linux-uclibc-4.9 ![test-result-svg][x32lu]
* x86_64-builder-linux-gnu-4.9 ![test-result-svg][x64lg]
* x86_64-builder-linux-musl-4.9 ![test-result-svg][x64lm]
* x86_64-builder-linux-uclibc-4.9 ![test-result-svg][x64lu]

machine tests
-------------

* aarch64-builder-linux-gnu ![test-result-svg][a64lg]
* armv5te926-builder-linux-gnueabihf ![test-result-svg][armv5]
* armv6j1136-builder-linux-gnueabihf ![test-result-svg][armv6]
* armv7a15-builder-linux-gnueabihf ![test-result-svg][armv7a15]
* armv7a9-builder-linux-gnueabihf ![test-result-svg][armv7a9]
* mips-builder-linux-gnu ![test-result-svg][m32lg]

sanitizers
----------

* -fsanitize=address ![test-result-svg][fsan]
* -fsanitize=leak ![test-result-svg][fsleak]
* -fsanitize=undefined ![test-result-svg][fsun]

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
options it is also valid to pass **--disable**. All enabled options can be later
disabled in runtime.

--enable-out-stderr (default: enable)
-------------------------------------

When set, library will be able to print logs to standard error output. Nothing
fancy.

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

--enable-timestamp
------------------

When enabled, logger will be able to add timestamp to every message. Timestamp
can be in short or long format and timer source can be configured. Check out
[man page](http://embedlog.kurwinet.pl/manuals/el_option.3.html) to read more
about it.

--enable-realtime, --enable-monotonic (default: enable)
-------------------------------------------------------

Allows to use better precision timers - **CLOCK_REALTIME** and
**CLOCK_MONOTONIC** but requires **POSIX**


--enable-finfo (default: enable)
--------------------------------

When enabled, information about line and file name from where log originated
will be added to each message.

--enable-colors (default: enable)
---------------------------------

If enabled, output logs can be colored depending on their level. Good for
quick error spotting.

--enable-reentrant (default: enable)
------------------------------------

Uses reentrant functions where possible. Not available on every platform, but
if enabled, provides thread-safety on line level - that means, lines won't
overlap with another thread

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
[LICENSE](http://git.kurwinet.pl/embedlog/tree/LICENSE) file for details

See also
========

* [c89 snprintf function family](https://www.ijs.si/software/snprintf) by
  Mark Martinec
* [mtest](http://mtest.kurwinet.pl) unit test framework **embedlog** uses
* [librb](http://librb.kurwinet.pl) ring buffer used in unit tests
* [git repository](http://git.kurwinet.pl/embedlog) to browse code online
* [continous integration](http://ci.embedlog.kurwinet.pl) for project
* [polarhome](http://www.polarhome.com) nearly free shell accounts for virtually
  any unix there is.

[a64lg]: http://ci.embedlog.kurwinet.pl/badges/aarch64-builder-linux-gnu-tests.svg
[armv5]: http://ci.embedlog.kurwinet.pl/badges/armv5te926-builder-linux-gnueabihf-tests.svg
[armv6]: http://ci.embedlog.kurwinet.pl/badges/armv6j1136-builder-linux-gnueabihf-tests.svg
[armv7a15]: http://ci.embedlog.kurwinet.pl/badges/armv7a15-builder-linux-gnueabihf-tests.svg
[armv7a9]: http://ci.embedlog.kurwinet.pl/badges/armv7a9-builder-linux-gnueabihf-tests.svg
[x32fb]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-freebsd-tests.svg
[x32lg]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-linux-gnu-tests.svg
[x32lm]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-linux-musl-tests.svg
[x32lu]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-linux-uclibc-tests.svg
[x32nb]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-netbsd-tests.svg
[x32ob]: http://ci.embedlog.kurwinet.pl/badges/i686-builder-openbsd-tests.svg
[m32lg]: http://ci.embedlog.kurwinet.pl/badges/mips-builder-linux-gnu-tests.svg
[x64lg]: http://ci.embedlog.kurwinet.pl/badges/x86_64-builder-linux-gnu-tests.svg
[x64lm]: http://ci.embedlog.kurwinet.pl/badges/x86_64-builder-linux-musl-tests.svg
[x64lu]: http://ci.embedlog.kurwinet.pl/badges/x86_64-builder-linux-uclibc-tests.svg
[x64ss]: http://ci.embedlog.kurwinet.pl/badges/x86_64-builder-solaris-tests.svg
[prhpux]: http://ci.embedlog.kurwinet.pl/badges/parisc-polarhome-hpux-tests.svg
[p4aix]: http://ci.embedlog.kurwinet.pl/badges/power4-polarhome-aix-tests.svg

[fsan]: http://ci.embedlog.kurwinet.pl/badges/fsanitize-address.svg
[fsleak]: http://ci.embedlog.kurwinet.pl/badges/fsanitize-leak.svg
[fsun]: http://ci.embedlog.kurwinet.pl/badges/fsanitize-undefined.svg
