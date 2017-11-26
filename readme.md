About
=====

Logger written in C89 targeting embedded systems. It's main goals are to be
small and fast. Many features can be disabled during compilation to generate
smaller binary. Ammount of needed stack memory can be lowered by adjusting
maximum printable size of single log message. Implemented features are:

  * printing to different outputs (simultaneously) like:
    - standard error (stderr)
    - file (with optional log rotate)
    - custom routine - can be anything **embedlog** just calls custom function
      with string to print
  * timestamp to every message
    - clock_t
    - time_t
    - CLOCK_REALTIME (requires POSIX)
    - CLOCK_MONOTONIC (requires POSIX)
  * print location of printed log (file and line)
  * 8 predefinied log levels and (sizeof(int) - 8) custom log levels ;-)
  * colorful output (ansi colors) for easy error spotting
  * print memory block in wireshark-like output

Almost all of these features can be disabled to save some precious bytes of
memory.

Dependencies
============

Library is written in C89 but some features require implemented POSIX to work.
Also there are some additional features for users with C99 compiler.

To run unit tests, you also need [librb][https://github.com/mlyszczek/librb]

License
=======

Library is licensed under BSD 2-clause license. See LICENSE file for details

Compiling and installing
========================

Project uses standard automake so to build you need to:

~~~
$ autoreconf -i
$ ./configure
$ make
# make install
~~~

Functions description
=====================

For detailed description of every function check out man pages.

Contact
=======

Michał Łyszczek <michal.lyszczek@bofc.pl>

Thanks to
=========

- Mark Martinec for nice C89 implementation of snprintf and familly
  (https://www.ijs.si/software/snprintf/)
- Myself for developing very simple unit test framework
(https://github.com/mlyszczek/mtest) for easy testing
- Myself for developing ring buffer used in unit tests
 (https://github.com/mlyszczek/librb)
