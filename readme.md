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
